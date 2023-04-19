/*
 * INFORMATION:
 * C++17
 * Compiler: gnu++17
 * Libraries: libsdl2-dev libsdl2-ttf-dev
 * Sysytem: Linux
 * Build: -g ./main.cc -lSDL2 -lSDL2main -lSDL2_ttf -std=c++17 -o main
 * Date: Mar 26, 2023
 * Time: 3.20am
 * 
 * BIBLIOGRAPHY:
 * https://github.com/mafm/HashLife
 * https://www.dev-mind.blog/hashlife/
 * https://github.com/ngmsoftware/hashlife
 */

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string>
#include <deque>
#include <map>

using std::map;
using std::cout;
using std::endl;
using std::stoi;
using std::deque;
using std::string;
using std::to_string;
using std::stringstream;

#define WIDTH 960
#define HEIGHT 720
const string ZERO = "0", ONE = "1";
const int P_VERTICAL = 0, P_HORIZONTAL = 1;

typedef deque<string> grid1D;
typedef deque<deque<string>> grid2D;

grid1D NewGrid1D(int l) {
    grid1D zeroes;
    for (int x = 0; x < l; x++)
        zeroes.push_back(ZERO);
    return zeroes;
}

grid2D NewGrid2D(int h, int w) {
    grid2D grid; grid1D zeroes;
    for (int x = 0; x < w; x++)
        zeroes.push_back(ZERO);
    for (int y = 0; y < h; y++)
        grid.push_back(zeroes);
    return grid;
}


template <typename int_t = int>
struct Vector2D {
    int_t x, y;

    Vector2D() {}

    Vector2D(int_t _x, int_t _y) : x(_x), y(_y) {}

    Vector2D operator=(const Vector2D &rhs) {
        Vector2D r(rhs.x, rhs.y);
        return r;
    }
};

void PrintArray(const grid2D &vx) {
    for (const auto &y : vx) {
        for (const auto &x : y)
            cout << x << " ";
        cout << endl;
    }
}

grid2D vx_rot90(grid2D &vx) {
    grid2D result;
    for (int x = 0; x < vx[0].size(); x++) {
        deque<string> row;
        for (int y = vx.size() - 1; y >= 0; y--) {
            row.push_back(vx[y][x]);
        }
        result.push_back(row);
    }
    return result;
}

grid2D vx_flip(grid2D &vx, int flip) {
    grid2D result;
    if (flip == P_VERTICAL) {
        for (int y = vx.size() - 1; y >= 0; y--) {
            result.push_back(vx[y]);
        }
    }
    else if (flip == P_HORIZONTAL) {
        for (int y = 0; y < vx.size(); y++)
        {
            deque<string> row;
            for (int x = vx[0].size(); x >= 0; x--) {
                row.push_back(vx[y][x]);
            }
            result.push_back(row);
        }
    }
    else throw std::invalid_argument("Flip must be -1, 0, or 1.");
    return result;
}

string str_trim(string &str) {
    if (str == "")
        return str;
    while (str != "" && str[0] == ' ')
    {
        str = str.substr(1);
    }
    while (str[str.length() - 1] == ' ')
    {
        str = str.substr(0, str.length() - 1);
    }
    return str;
}

deque<string> str_split(string &str, string delim) {
    int dlen = delim.length();
    deque<string> result;
    string buffer = "";
    for (int x = 0; x < str.length();)
    {
        if (str.substr(x, dlen) == delim)
        {
            result.push_back(buffer);
            x = x + dlen;
            buffer = "";
        }
        else
        {
            buffer = buffer + str[x];
            x++;
        }
    }
    if (buffer != "")
        result.push_back(buffer);
    return result;
}

namespace ReadPattern {
    map<string, grid2D> cache = {};
    const map<char, int> STREAM = {
        {'.', 0}, {'O', 1}};

    deque<string> ReadFile(string addr) {
        string line;
        deque<string> result;
        std::ifstream file(addr, std::ios::in);
        while (std::getline(file, line))
        {
            line = str_trim(line);
            result.push_back(line);
        }
        file.close();
        return result;
    }

    // Reads a pattern from cache and augments it appropriately
    grid2D read_from_cache(string addr, int angle, int flip) {
        grid2D pattern = cache[addr];
        if (angle > 0)
            pattern = vx_rot90(pattern);
        if (angle > 90)
            pattern = vx_rot90(pattern);
        if (angle > 180)
            pattern = vx_rot90(pattern);
        return (flip != -1) ? vx_flip(pattern, flip) : pattern;
    }

    // Reads a pattern from file and saves it to the cache
    grid2D read_from_dir(string addr, int angle, int flip) {
        cache[addr] = grid2D(); // Add new entry to the cache
        auto pattern = ReadFile("./src/" + addr);
        for (const string &line : pattern) {
            // Check if line is a comment
            if (line[0] == '#' || line == "")
                continue;
            if (line[0] == '.' || line[0] == 'O')
            {
                deque<string> row;
                for (const char &x : line)
                {
                    if (x == '.')
                        row.push_back(ZERO);
                    else if (x == 'O')
                        row.push_back(ONE);
                    else
                        throw std::invalid_argument("Unknown char");
                }
                cache[addr].push_back(row);
            }
        }
        return read_from_cache(addr, angle, flip);
    }

    // Reads returns a pattern
    grid2D parser_load(string addr, int angle, int flip) {
        //  Search the cache for preloaded
        if (cache.find(addr) != cache.end())
        {
            return read_from_cache(addr, angle, flip);
        }
        // Search currently directory for file
        return read_from_dir(addr, angle, flip);
    }

    // Reads a .configuration file
    grid2D Load_Config_File(string addr) {
        grid2D grid;
        int w = 0, h = 0;
        string pattern = "";
        int angle = 0, flip = -1;
        Vector2D place = {-1, -1};
        Vector2D anchor = {-1, -1};

        // Read the contents of the file
        auto script = ReadFile(addr);
        for (const string &line : script) {
            // Check if line is a comment
            if (line[0] == '#' || line == "")
                continue;
            // Check if to load and append pattern to grid
            if (line.substr(0, 5) == "[end]") {
                if (pattern == "grid") {
                    // Fill in grid with zeroes
                    grid = NewGrid2D(h, w);
                }
                else {
                    // Place pattern at position
                    grid2D array = parser_load(pattern, angle, flip);
                    // Get the size of the array
                    int sy = array.size(), sx = array[0].size();
                    // Configure start and end positions for the pattern on the grid
                    Vector2D pos = {place.x - sx * anchor.x, place.y - sy * anchor.y};
                    // Place the pattern on the grid
                    for (int my = 0; my < sy; my++)
                    {
                        for (int mx = 0; mx < sx; mx++)
                        {
                            // Can sometimes erase other patterns if all is copied.!!
                            if (array[my][mx] == ONE)
                                grid[pos.y + my][pos.x + mx] = array[my][mx];
                        }
                    }
                }
                pattern = "";
                angle = 0, flip = -1;
                place.x = -1, place.y = -1;
                anchor.x = -1, anchor.y = -1;
            }
            else if (line.substr(0, 2) == "W:")
            {
                string x = line.substr(2); // Remove "W:" from the line
                w = stoi(str_trim(x));     // Remove spaces and convert to int
            }
            else if (line.substr(0, 2) == "H:")
            {
                string x = line.substr(2); // Remove "H:" from the line
                h = stoi(str_trim(x));     // Remove spaces and convert to int
            }
            else if (line[0] == '[' && line[line.length() - 1] == ']')
            {
                // Make sure it is starting a new block
                if (pattern != "")
                    throw std::invalid_argument("Missing [end] of block");
                pattern = line.substr(1); // Remove the enclosing brackets
                pattern = pattern.substr(0, pattern.length() - 1);
            }
            else if (line.substr(0, 6) == "angle:")
            {
                string x = line.substr(6); // Remove "angle:" from the line
                angle = stoi(str_trim(x)); // Remove spaces and convert to int
                // Make sure it is a multiple of 90
                angle = int((angle % 360) / 90) * 90;
            }
            else if (line.substr(0, 5) == "flip:")
            {
                string x = line.substr(5); // Remove "flip:" from the line
                x = str_trim(x);           // Trip the spaces
                //  Mark as HORIZONTAL, VERTICAL or No flip
                if (x == "HORIZONTAL")
                    flip = P_HORIZONTAL;
                else if (x == "VERTICAL")
                    flip = P_VERTICAL;
                else
                    flip = -1;
            }
            else if (line.substr(0, 6) == "place:")
            {
                string x = line.substr(6);       // Remove "place:" from the line
                x = str_trim(x);                 // Trip the spaces
                auto _place = str_split(x, ","); // Split by the comma separator
                // Place must be a 2D deque, it is inverted i.e. (y, x) not (x, y)
                if (_place.size() != 2)
                    throw std::invalid_argument("Unable to append place");
                place.x = stoi(_place[1]), place.y = stoi(_place[0]);
            }
            else if (line.substr(0, 7) == "anchor:")
            {
                string x = line.substr(7);        // Remove "anchor:" from the line
                x = str_trim(x);                  // Trip the spaces
                auto _anchor = str_split(x, ","); // Split by the comma separator
                // Anchor must be a 2D deque, it is inverted i.e. (y, x) not (x, y)
                if (_anchor.size() != 2)
                    throw std::invalid_argument("Unable to append anchor");
                anchor.x = stoi(_anchor[1]), anchor.y = stoi(_anchor[0]);
            }
        }
        return grid;
    }
}

namespace GameOfLife {
    struct _QTreeNode_ {
        int depth, area;
        string nw, ne, sw, se;
    };

    struct _QLeaflet_ {
        string n;
        Vector2D<> r1, r2;
        _QLeaflet_(string _n, Vector2D<> _r1, Vector2D<> _r2):
            n(_n), r1(_r1), r2(_r2) {}

        _QLeaflet_ operator=(const _QLeaflet_ &rhs) {
            _QLeaflet_ r(rhs.n, rhs.r1, rhs.r2);
            return r;
        }
    };

    struct _QConfigure_ {
        Vector2D<> size;
        map<int, map<int, string>> array;

        _QConfigure_() {}
        _QConfigure_(int s) {
            size.x = s, size.y = s;
        }
        _QConfigure_(Vector2D<> s) {
            size.x = s.x, size.y = s.y;
        }

        bool find(int a, int b) {
            // First check if "a" is in the map
            if (array.find(a) == array.end()) return false;
            if (array[a].find(b) == array[a].end()) return false;
            return true;
        }
    };

    long int address = 99;              // Unique address for the Nodes
    long int GENERATION = 0;            // Keep track of the number of generations
    string Configuration = "";          // The current configuration
    map<string, _QTreeNode_> NodeList;  // A list of all referenced nodes

    // Checks if two nodes are equal by comparing their quadrants.
    // It recursively checks if the quadrants are equivalent...
    // until it gets to its base values ALIVE(1), DEAD(0)
    const bool _Eq_Node_(_QTreeNode_ &lhs, _QTreeNode_ &rhs) {
        if ((lhs.area != rhs.area) || (lhs.depth != rhs.depth))
            return false;
        return (
            lhs.nw == rhs.nw && lhs.ne == rhs.ne &&
            lhs.sw == rhs.sw && lhs.se == rhs.se);
    }

    // Checks if two nodes are equal using their address.
    // It points to the TreeNode overload function.
    const bool _Eq_Node_(const string &lhs, const string &rhs) {
        return _Eq_Node_(NodeList.at(lhs), NodeList.at(rhs));
    }

    // Finds the address for any node
    const string FindNode(_QTreeNode_ &node) {
        // Make sure that the node is legitimate.
        assert(node.nw != "");
        assert(node.ne != "");
        assert(node.sw != "");
        assert(node.se != "");
        assert(node.depth != 0);
        // Find the key to the node
        for (auto &[k, n] : NodeList) {
            if (_Eq_Node_(n, node))
                return k;
        }
        return "";
    }

    // Adds and initialises the new node.
    // Returns the address of the new node;
    const string AddNode(_QTreeNode_ &node) {
        string addr = FindNode(node);
        // Create a new node if it does not exist
        if (addr == "") {
            address++;
            addr = to_string(address);
            NodeList[addr] = node;
        }
        return addr;
    }

    // Uses bit int values to generate a canonical node
    const string NewNode(int nw, int ne, int sw, int se) {
        _QTreeNode_ newnode;
        newnode.area = nw + ne + sw + se;
        newnode.nw = to_string(nw);
        newnode.ne = to_string(ne);
        newnode.sw = to_string(sw);
        newnode.se = to_string(se);
        newnode.depth = 1;
        return AddNode(newnode);
    }

    // Uses node references to generate a new node, or...
    // uses bit char to generate a canonical node
    const string NewNode(string &nw, string &ne, string &sw, string &se) {
        // Check and handle when they are ALIVE(1) or DEAD(0) cells
        if ((nw == "1" || nw == "0") && (ne == "1" || ne == "0")) {
            if ((sw == "1" || sw == "0") && (se == "1" || se == "0")) {
                return NewNode((nw == "1") ? 1 : 0, (ne == "1") ? 1 : 0, (sw == "1") ? 1 : 0, (se == "1") ? 1 : 0);
            }
        }
        // Make sure that they are all of the same depth
        if (NodeList.at(nw).depth != NodeList.at(ne).depth)
            throw std::invalid_argument("Must be the same depth");
        if (NodeList.at(ne).depth != NodeList.at(sw).depth)
            throw std::invalid_argument("Must be the same depth");
        if (NodeList.at(sw).depth != NodeList.at(se).depth)
            throw std::invalid_argument("Must be the same depth");
        if (NodeList.at(nw).depth == 0)
            throw std::invalid_argument("Depth must be greater than one");

        _QTreeNode_ newnode;
        newnode.nw = nw;
        newnode.ne = ne;
        newnode.sw = sw;
        newnode.se = se;
        newnode.depth = NodeList.at(nw).depth + 1;
        newnode.area = NodeList.at(nw).area + NodeList.at(ne).area;
        newnode.area += NodeList.at(sw).area + NodeList.at(se).area;
        return AddNode(newnode);
    }

    // Returns the center cell when aligned horizontally
    const string CenteredHorizontal(string &west, string &east) {
        _QTreeNode_ &W = NodeList.at(west), &E = NodeList.at(east);
        return NewNode(
            NodeList.at(W.ne).se, NodeList.at(E.nw).sw,
            NodeList.at(W.se).ne, NodeList.at(E.sw).nw);
    }

    // Returns the center cell when aligned vertically
    const string CenteredVertical(string &north, string &south) {
        _QTreeNode_ &N = NodeList.at(north), &S = NodeList.at(south);
        return NewNode(
            NodeList.at(N.sw).se, NodeList.at(N.se).sw,
            NodeList.at(S.nw).ne, NodeList.at(S.ne).nw);
    }

    // Returns the center cell when centered
    const string CenteredSubNode(string &node) {
        _QTreeNode_ &n = NodeList.at(node);
        return NewNode(
            NodeList.at(n.nw).se, NodeList.at(n.ne).sw,
            NodeList.at(n.sw).ne, NodeList.at(n.se).nw);
    }

    // Returns the center cell when centered x2
    const string CenteredSubSubNode(string &node) {
        _QTreeNode_ &n = NodeList.at(node);
        return NewNode(
            NodeList.at(NodeList.at(n.nw).se).se, NodeList.at(NodeList.at(n.ne).sw).sw,
            NodeList.at(NodeList.at(n.sw).ne).ne, NodeList.at(NodeList.at(n.se).nw).nw);
    }

    // Applies Conway's Game of Life rules
    const string TheRules(string &node) {
        auto Rule = [](const char cell, const int alive) {
            if (cell == 0 && alive == 3)
                return 1; // Dead cell revives: Reproduction
            if (cell == 1 && (alive == 2 || alive == 3))
                return 1; // Alive cell survives
            return 0;     // Over-/Under-population
        };

        _QTreeNode_ &n = NodeList.at(node);
        if (n.depth != 2)
            throw std::invalid_argument("Unexpected depth");
        deque<deque<int>> M = {
            {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};
        M[1][1] = stoi(NodeList.at(n.nw).nw), M[1][2] = stoi(NodeList.at(n.nw).ne),
        M[1][3] = stoi(NodeList.at(n.ne).nw), M[1][4] = stoi(NodeList.at(n.ne).ne),
        M[2][1] = stoi(NodeList.at(n.nw).sw), M[2][2] = stoi(NodeList.at(n.nw).se),
        M[2][3] = stoi(NodeList.at(n.ne).sw), M[2][4] = stoi(NodeList.at(n.ne).se),
        M[3][1] = stoi(NodeList.at(n.sw).nw), M[3][2] = stoi(NodeList.at(n.sw).ne),
        M[3][3] = stoi(NodeList.at(n.se).nw), M[3][4] = stoi(NodeList.at(n.se).ne),
        M[4][1] = stoi(NodeList.at(n.sw).sw), M[4][2] = stoi(NodeList.at(n.sw).se),
        M[4][3] = stoi(NodeList.at(n.se).sw), M[4][4] = stoi(NodeList.at(n.se).se);
        return NewNode(
            Rule(M[2][2], M[1][1] + M[1][2] + M[1][3] + M[2][1] + M[2][3] + M[3][1] + M[3][2] + M[3][3]),
            Rule(M[2][3], M[1][2] + M[1][3] + M[1][4] + M[2][2] + M[2][4] + M[3][2] + M[3][3] + M[3][4]),
            Rule(M[3][2], M[2][1] + M[2][2] + M[2][3] + M[3][1] + M[3][3] + M[4][1] + M[4][2] + M[4][3]),
            Rule(M[3][3], M[2][2] + M[2][3] + M[2][4] + M[3][2] + M[3][4] + M[4][2] + M[4][3] + M[4][4]));
    }

    // Returns a stack of C0000 canonical
    const string stack_C0000(int depth) {
        if (depth == 0)
            throw std::invalid_argument("Too deep to handle");
        string zeros = NewNode(0, 0, 0, 0);
        for (int x = 2; x < depth + 1; x++)
        {
            zeros = NewNode(zeros, zeros, zeros, zeros);
        }
        return zeros;
    }

    // Appropriately pads a node
    const string PadNode(string &node) {
        _QTreeNode_ &n = NodeList.at(node);
        string border = stack_C0000(n.depth - 1);
        string reNW = NewNode(border, border, border, n.nw);
        string reNE = NewNode(border, border, n.ne, border);
        string reSW = NewNode(border, n.sw, border, border);
        string reSE = NewNode(n.se, border, border, border);
        return NewNode(reNW, reNE, reSW, reSE);
    }

    // Removes excess zero leafs
    const string StripNode(string &node) {
        while (true) {
            // Limit split up to the first layer layer
            if (NodeList.at(node).depth == 1) break;
            string center = CenteredSubNode(node);
            if (NodeList.at(node).area == NodeList.at(center).area)
                node = center;
            else break;
        }
        return node;
    }

    // Reads the node as a 2x2 matrix
    _QConfigure_ ReadConfigurationAsMap() {
        _QConfigure_ config(
            pow(2, NodeList.at(Configuration).depth)
        );
        { // Remove all zeroes
            deque<_QLeaflet_> buffer, rebuff;
            string zero = NewNode(0, 0, 0, 0);
            buffer = {{Configuration, {1, 1}, config.size}};
            auto _Add = [&](string &n, Vector2D<int> &c1, Vector2D<int> &c2, bool buffer) {
                if (NodeList.at(n).area == 0)
                    return; // Does nothing if the node is all zero
                // Check if the depth is reasonable
                if (NodeList.at(n).depth == 0)
                    throw std::invalid_argument("Depth must be above 0");
                // Adds everything else to the buffer
                if (buffer)
                    return rebuff.push_back({n, c1, c2});
                // Checks if the block is not deep enough
                if (NodeList.at(n).depth != 1)
                    throw std::invalid_argument("Uncanonical of depth" + to_string(NodeList.at(n).depth));
                // Maps the entirety of the canonical node
                Vector2D r = c1;
                auto m = NodeList.at(n);
                r.x = r.x - 1, r.y = r.y - 1;
                config.array[r.y][r.x] = m.nw;
                config.array[r.y][r.x + 1] = m.ne;
                config.array[r.y + 1][r.x] = m.sw;
                config.array[r.y + 1][r.x + 1] = m.se;
            };
            // Get the alive parts of the array as canonical
            while (buffer.size() > 0) {
                rebuff.clear();
                for (_QLeaflet_ &b : buffer) {
                    auto n = NodeList.at(b.n);
                    // Divide the section into nw, ne, sw, se by index
                    Vector2D<int> r3 = {
                        (b.r2.x + b.r1.x - 1) / 2,
                        (b.r2.y + b.r1.y - 1) / 2};
                    Vector2D<int> nw1 = b.r1, nw2 = r3;
                    Vector2D<int> ne1 = {r3.x + 1, b.r1.y}, ne2 = {b.r2.x, r3.y};
                    Vector2D<int> sw1 = {b.r1.x, r3.y + 1}, sw2 = {r3.x, b.r2.y};
                    Vector2D<int> se1 = {r3.x + 1, r3.y + 1}, se2 = b.r2;
                    // Append to list for further recursion
                    if (NodeList.at(b.n).depth > 2) {
                        _Add(NodeList.at(b.n).nw, nw1, nw2, true);
                        _Add(NodeList.at(b.n).ne, ne1, ne2, true);
                        _Add(NodeList.at(b.n).sw, sw1, sw2, true);
                        _Add(NodeList.at(b.n).se, se1, se2, true);
                    }
                    else if (NodeList.at(b.n).depth == 2) {
                        _Add(NodeList.at(b.n).nw, nw1, nw2, false);
                        _Add(NodeList.at(b.n).ne, ne1, ne2, false);
                        _Add(NodeList.at(b.n).sw, sw1, sw2, false);
                        _Add(NodeList.at(b.n).se, se1, se2, false);
                    }
                    else if (NodeList.at(b.n).depth == 1) {
                        _Add(b.n, nw1, nw2, false);
                    }
                }
                buffer.clear();
                buffer = rebuff;
            }
        }

        return config;
    }

    // Computes the next generation of the grid
    const string NextGeneration(string &node) {
        _QTreeNode_ &n = NodeList.at(node);

        if (n.area == 0) return CenteredSubNode(node);
        if (n.depth == 2) return TheRules(node);

        // Generate the auxiliary nodes
        string node11 = CenteredSubNode(n.nw);
        string node12 = CenteredHorizontal(n.nw, n.ne);
        string node13 = CenteredSubNode(n.ne);
        string node21 = CenteredVertical(n.nw, n.sw);
        string node22 = CenteredSubSubNode(node);
        string node23 = CenteredVertical(n.ne, n.se);
        string node31 = CenteredSubNode(n.sw);
        string node32 = CenteredHorizontal(n.sw, n.se);
        string node33 = CenteredSubNode(n.se);

        string aux11 = NewNode(node11, node12, node21, node22);
        string aux12 = NewNode(node12, node13, node22, node23);
        string aux21 = NewNode(node21, node22, node31, node32);
        string aux22 = NewNode(node22, node23, node32, node33);

        aux11 = NextGeneration(aux11);
        aux12 = NextGeneration(aux12);
        aux21 = NextGeneration(aux21);
        aux22 = NextGeneration(aux22);

        _QTreeNode_ &a11 = NodeList.at(aux11);
        _QTreeNode_ &a12 = NodeList.at(aux12);
        _QTreeNode_ &a21 = NodeList.at(aux21);
        _QTreeNode_ &a22 = NodeList.at(aux22);
        return NewNode(aux11, aux12, aux21, aux22);
    }

    void NextGeneration() {
        GENERATION++;       // Increase generations past
        // First pad because fn returns half the block size
        Configuration = PadNode(Configuration);
        // This is only used for outside calls...
        // Other function is called recursively
        Configuration = NextGeneration(Configuration);
        Configuration = StripNode(Configuration);
    }

    // Loads and places the .config file in a node
    void Load_Config_To_Node(string addr) {
        // Load the .config file as a grid
        auto grid = ReadPattern::Load_Config_File(addr);
        {
            cout << "[configuration] is ready" << endl;
            // Get the resolution(size) of the grid
            Vector2D res = {grid[0].size(), grid.size()};
            cout << "Grid: w[" << res.x << "] h[" << res.y << "]" << endl;
            // Calculate the order of magnitude of the grid
            int k = std::max(res.x, res.y); // The bigger of the two sides
            k = ceil(log2(k));              // The minimum order of magnitude i.e. 2^k
            k = pow(2, k);                  // The true grid [quad tree] size
            cout << "Quadtree: " << k << endl;
            // Place the grid on the quad tree block [tru grid]
            Vector2D shift = {(k - res.x) / 2, (k - res.y) / 2};
            Vector2D remnant = {k - shift.x - res.x, k - shift.y - res.y};
            cout << "Padding grid to fit quadtree" << endl;
            // Expand the width of the grid...
            // by filling with chunks of zeroes on both sides
            for (int y = 0; y < res.y; y++) {
                for (int x = 0; x < shift.x; x++) grid[y].push_front(ZERO);
                for (int x = 0; x < remnant.x; x++) grid[y].push_back(ZERO);
            }
            // Expand the height of the grid...
            // by filling with deque of zeroes on both sides
            deque<string> zeroes = NewGrid1D(k);
            for (int y = 0; y < shift.y; y++) grid.push_front(zeroes);
            for (int y = 0; y < remnant.y; y++) grid.push_back(zeroes);
            cout << "Grid: w[" << grid[0].size() << "] h[" << grid.size() << "]" << endl;
        }

        // Convert the grid to a tree node
        while (grid.size() > 2 && grid[0].size() > 2) {
            deque<deque<string>> newgrid;
            for (int y = 0; y < grid.size(); y = y + 2) {
                deque<string> newrow;
                for (int x = 0; x < grid[0].size(); x = x + 2) {
                    newrow.push_back(NewNode(
                        grid[y][x], grid[y][x + 1],
                        grid[y + 1][x], grid[y + 1][x + 1]));
                }
                newgrid.push_back(newrow);
            }   grid = newgrid;
        }
        // Create the final node and remove excess blocks
        Configuration = NewNode(
            grid[0][0], grid[0][1], grid[1][0], grid[1][1]
        );
        // Remove excess borders from the configuration
        Configuration = StripNode(Configuration);
        cout << "Depth: " << NodeList.at(Configuration).depth << endl;
        cout << "Area: " << NodeList.at(Configuration).area << endl;
        cout << "Configuration: " << Configuration << endl;
    }
}

namespace Text {
    TTF_Font *Font = NULL;
    SDL_Rect TextRect = {};
    SDL_Surface *TextSurface = NULL;
    SDL_Texture *TextTexture = NULL;
    SDL_Color TextColor = {255, 255, 255, 255};

    void RenderText(SDL_Renderer *renderer, string msg) {
        TTF_Init();

        Font = TTF_OpenFont("./fonts/07558_CenturyGothic.ttf", 12);
        if (!Font) {
            cout << TTF_GetError() << endl;
            cout << "TTF font unloaded" << endl;
            return;
        }
        TextSurface = TTF_RenderText_Solid(Font, msg.c_str(), TextColor);
        if (!TextSurface) {
            cout << TTF_GetError() << endl;
            cout << "TTF unable to create surface" << endl;
            return;
        }
        TextTexture = SDL_CreateTextureFromSurface(renderer, TextSurface);
        if (!TextTexture) {
            cout << TTF_GetError() << endl;
            cout << "TTF unable to create texture" << endl;
            return;
        }

        TextRect.x = 0;
        TextRect.y = 0;
        TextRect.w = TextSurface->w;
        TextRect.h = TextSurface->h;
        SDL_FreeSurface(TextSurface);
        SDL_RenderCopy(renderer, TextTexture, NULL, &TextRect);

        TTF_Quit();
    }

    void Quit() {
        SDL_DestroyTexture(TextTexture);
        Font = NULL;
        TextRect = {};
        TextSurface = NULL;
        TextTexture = NULL;
    }
}

namespace STATUS {
    SDL_Point gridpos;
    SDL_Point mousepos;
    stringstream stream;
    string mousewhere = "";
    SDL_Rect ViewPort = {0, HEIGHT - 20, WIDTH, 20};

    void RenderStatus(SDL_Renderer *renderer) {
        SDL_RenderSetViewport(renderer, &ViewPort);

        stream.str(string()); // Clear the stream
        // Add the grid coordinates
        stream << "x: " << gridpos.x << ", ";
        stream << "y: " << gridpos.y;
        // Add a tab after the grid coordinates
        stream << "  |  ";
        stream << "Generation: " << GameOfLife::GENERATION;
        Text::RenderText(renderer, stream.str());
    }
}

namespace CAM {
    int zoom = 0;
    int ppc = zoom + 1.;              // Pixel per cell
    int pps = zoom + 2.;              // Pixel per skip
    int block = 10 + (ppc * 10);      // Size of cells 1 to 10
    SDL_Rect Cell = {0, 0, pps, pps}; // The rect of a single cell
    SDL_Rect ViewPort = {0, 0, WIDTH, HEIGHT - 20};
    namespace position {
        int x = 0, y = 0;
    }
    namespace mid {
        int w = ViewPort.w / 2, h = ViewPort.h / 2;
    }
    float scale_w = ceil(float(mid::w - 1.) / block);
    float scale_h = ceil(float(mid::h - 1.) / block);

    void Recalculate() {
        ppc = zoom + 1;
        pps = zoom + 2.;
        block = 10 + (ppc * 10);
        Cell.w = pps, Cell.h = pps;
    }
    void ZoomIn() {
        zoom++;
        zoom = (zoom > ViewPort.h)? ViewPort.h: zoom;
        Recalculate();
    }
    void ZoomOut() {
        zoom--;
        zoom = (zoom < 0)? 0: zoom;
        Recalculate();
    }

    SDL_Point GetGridCoordinates(const SDL_Point &mpos) {
        float x = float(mpos.x - mid::w - position::x) / pps;
        float y = float(mid::h + position::y - mpos.y) / pps;
        return {
            int(x + ((x < 0) ? -1 : 1)),
            int(y + ((y < 0) ? -1 : 1))
        };
    }

    void RenderGrid(SDL_Renderer *renderer) {
        SDL_RenderSetViewport(renderer, &ViewPort);

        // Calculating starting points, wrap around the block
        float start_w = mid::w - (scale_w * block) - 1;
        start_w = start_w + (position::x % block);
        start_w -= (start_w > 0) ? ceil(start_w / block) * block : 0;

        float start_h = mid::h - (scale_h * block) - 1;
        start_h = start_h + (position::y % block);
        start_h -= (start_h > 0) ? ceil(start_h / block) * block : 0;

        // Curtain Sweep: Width and Height
        SDL_SetRenderDrawColor(renderer, 63, 63, 63, 255);
        for (int w = start_w; w < ViewPort.w; w = w + pps) {
            if (w < 0) continue;
            SDL_RenderDrawLine(renderer, w, 0, w, ViewPort.h - 1);
        }
        for (int h = start_h; h < ViewPort.h; h = h + pps) {
            if (h < 0) continue;
            SDL_RenderDrawLine(renderer, 0, h, ViewPort.w - 1, h);
        }

        // Curtain Sweep: Highlights
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        for (int w = start_w; w < ViewPort.w; w = w + block) {
            if (w < 0) continue;
            SDL_RenderDrawLine(renderer, w, 0, w, ViewPort.h - 1);
        }
        for (int h = start_h; h < ViewPort.h; h = h + block) {
            if (h < 0) continue;
            SDL_RenderDrawLine(renderer, 0, h, ViewPort.w - 1, h);
        }

        // Curtian Sweep: Fill Cell
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        auto config = GameOfLife::ReadConfigurationAsMap();
        auto sy = config.size.y, sx = config.size.x; // Size of the array
        auto hy = sy / 2, hx = sx / 2;               // Half size
        for (int h = start_h; h < ViewPort.h; h = h + pps) {
            for (int w = start_w; w < ViewPort.w; w = w + pps) {
                if (w < 0 || h < 0) continue;
                // Convert the position to grid coordinates
                SDL_Point cell = GetGridCoordinates({w, h});
                // Convert the coordinates to array position
                Vector2D acell = {
                    hx + cell.x - ((cell.x < 0) ? 0 : 1),
                    hy - cell.y - ((cell.y > 0) ? 0 : 1)};
                // Make sure they are within the size of the array
                if (acell.x < 0 || acell.x >= sx) continue;
                if (acell.y < 0 || acell.y >= sy) continue;
                // Check if the cell of the array contains a ONE
                if (!config.find(acell.y, acell.x)) continue;
                if (config.array[acell.y][acell.x] == ONE) {
                    Cell.y = h - pps; // Fill the information in the Cell rect obj...
                    Cell.x = w - pps; // at the current pointer to the grid(w, h)
                    SDL_RenderFillRect(renderer, &Cell);
                }
            }
        }
    }

}

string FindViewPort() {
    if (SDL_PointInRect(&STATUS::mousepos, &CAM::ViewPort))
        return "CAM";
    if (SDL_PointInRect(&STATUS::mousepos, &STATUS::ViewPort))
        return "STATUS";
    return "";
}

int main(int argc, char **argv) {
    cout << "始まる..." << endl;
    // GAME OF LIFE: Initialise Canonical values
    cout << "「Basic Canonical Nodes」って言う物を作る" << endl;
    GameOfLife::NewNode(0, 0, 0, 0);
    GameOfLife::NewNode(0, 0, 0, 1);
    GameOfLife::NewNode(0, 0, 1, 0);
    GameOfLife::NewNode(0, 0, 1, 1);
    GameOfLife::NewNode(0, 1, 0, 0);
    GameOfLife::NewNode(0, 1, 0, 1);
    GameOfLife::NewNode(0, 1, 1, 0);
    GameOfLife::NewNode(0, 1, 1, 1);
    GameOfLife::NewNode(1, 0, 0, 0);
    GameOfLife::NewNode(1, 0, 0, 1);
    GameOfLife::NewNode(1, 0, 1, 0);
    GameOfLife::NewNode(1, 0, 1, 1);
    GameOfLife::NewNode(1, 1, 0, 0);
    GameOfLife::NewNode(1, 1, 0, 1);
    GameOfLife::NewNode(1, 1, 1, 0);
    GameOfLife::NewNode(1, 1, 1, 1);
    // Load the .configuration pattern
    cout << "「[configuration]」って言うファイルを読み込む" << endl;
    GameOfLife::Load_Config_To_Node("./[configuration]");

    // Initialise Simple Direct Media Layer
    cout << "「SDL」を始める" << endl;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
        "「ゲームの生活」を遊ぶ",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        WIDTH, HEIGHT,
        SDL_WINDOW_BORDERLESS
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    // The Application Loop ---------------------------------------------------
    SDL_Event event;
    bool QUIT = false, MOUSE1_PRIMED = false;
    cout << "[入っている: Application Loop]" << endl;
    while (!QUIT) {
        // Polling Event ------------------------------------------------------
        while(SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) QUIT = true;
            else if (event.type == SDL_MOUSEMOTION) {
                STATUS::mousepos = {event.motion.x, event.motion.y};
                STATUS::mousewhere = FindViewPort();
                STATUS::gridpos = CAM::GetGridCoordinates(STATUS::mousepos);
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.x < 0 || event.wheel.y < 0) {
                    CAM::ZoomOut();
                }
                else if (event.wheel.x > 0 || event.wheel.y > 0) {
                    CAM::ZoomIn();
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (FindViewPort() != "CAM") break;
                    MOUSE1_PRIMED = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (!MOUSE1_PRIMED) break;
                    if (FindViewPort() != "CAM") break;
                    if (STATUS::mousepos.x != event.motion.x) break;
                    if (STATUS::mousepos.y != event.motion.y) break;
                    MOUSE1_PRIMED = false;
                    cout << "TODO: for mouse click" << endl;
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                // Check the SDLKey values and move change the coords
                if (event.key.keysym.sym == SDLK_LEFT) CAM::position::x -= 1;
                else if (event.key.keysym.sym == SDLK_UP) CAM::position::y -= 1;
                else if (event.key.keysym.sym == SDLK_DOWN) CAM::position::y += 1;
                else if (event.key.keysym.sym == SDLK_RIGHT) CAM::position::x += 1;
            }
        }

        // Rendering to Window ------------------------------------------------
        // Wipe the renderer w black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // Draw the grid and the ALIVE pixels
        CAM::RenderGrid(renderer);
        // Show status.. idk.!!
        STATUS::RenderStatus(renderer);

        // Present render to Window
        SDL_RenderPresent(renderer);

        // Compute the next generation of Life
        GameOfLife::NextGeneration();
    }
    cout << "[出っている: Application Loop]" << endl;

    // cleanup SDL
    Text::Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
