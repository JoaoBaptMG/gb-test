#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <array>
#include "nlohmann/json.hpp"
#include "char.hpp"

struct State
{
    std::vector<Character> chars;
    std::map<Character, int> charIndices;
    std::vector<std::array<int, 4>> tiles;

    int addCharacter(const Character& character)
    {
        auto it = charIndices.find(character);
        if (it != charIndices.end())
            return it->second;
        auto sz = chars.size();
        charIndices.emplace(character, sz);
        chars.push_back(character);
        return sz;
    }
};

struct Offset { int x, y; };

int processAutotile(State& state, std::string name);
int processTileset(State& state, std::string name);

inline static nlohmann::json get(const nlohmann::json& j, const std::string& key)
{
    if (j.contains(key)) return j.at(key);
    return nlohmann::json();
}

int tilemapExport(int argc, char **argv)
{
    std::string in = argv[2];
    std::string out = argv[3];

    auto inlabel = labelizeName(in);

    auto fid = in.find_last_of('/');
    auto folder = fid == std::string::npos ? "" : in.substr(0, fid) + '/';

    std::ifstream inf(in);
    nlohmann::json j;
    inf >> j;

    // Get the main attributes
    auto at0 = get(j, "autotile0");
    auto at1 = get(j, "autotile1");
    auto tset = get(j, "tileset");

    State state;
    int emptyid = state.addCharacter(Character());
    state.tiles.push_back({ emptyid, emptyid, emptyid, emptyid });

    int r;
    if (at0.is_string() && (r = processAutotile(state, folder + at0.get<std::string>())) != 0) return r;
    if (at1.is_string() && (r = processAutotile(state, folder + at1.get<std::string>())) != 0) return r;
    if (tset.is_string() && (r = processTileset(state, folder + tset.get<std::string>())) != 0) return r;

    // Check validity
    if (state.chars.size() > 256)
    {
        std::cout << "Error! Maximum number of generated characters for tileset " << in << " exceeded (256).";
        return -1;
    }

    if (state.tiles.size() > 128)
    {
        std::cout << "Error! Maximum number of generated tiles for tileset " << in << " exceeded (128).";
        return -1;
    }

    state.tiles.resize(128);

    std::ofstream of(out);
    of << "; " << std::endl;
    of << "; " << out << std::endl;
    of << "; " << std::endl << std::endl;

    of << "section \"tileset metadata " << out <<  "\", rom0" << std::endl;
    of << inlabel << "::" << std::endl;
    of << "    db BANK(" << inlabel << "_chars), LOW(" << inlabel << "_chars), HIGH(" << inlabel << "_chars)" << std::endl;
    of << "    dw " << (state.chars.size() * CharSize) << std::endl;
    of << "    db BANK(" << inlabel << "_tiles), HIGH(" << inlabel << "_tiles)" << std::endl << std::endl;

    of << "section \"chars " << out <<  "\", romx" << std::endl;
    of << inlabel << "_chars:" << std::endl;
    for (auto& charac : state.chars) of << charac << std::endl;
    of << std::endl;

    of << "section \"tile data " << out <<  "\", romx, align[8]" << std::endl;
    of << inlabel << "_tiles:";
    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < 128; i++)
        {
            if (i % 16 == 0) of << std::endl << "    db ";
            else of << ", ";
            of << state.tiles[i][j];
        }
        of << std::endl;
    }

    return 0;
}

extern Offset autotileOffsets[47][4];

int processAutotile(State& state, std::string name)
{
    util::grid<Character> chars;
    int error = ConvertPngToCharacters(chars, name);
    if (error)
    {
        std::cout << "Error while loading autotile " << name << ": ";
        std::cout << lodepng_error_text(error) << std::endl;
        return error;
    }

    if (chars.width() != 4 || chars.height() != 6)
    {
        std::cout << "Autotile " << name << " must be 32x48!" << std::endl;
        return -1;
    }

    for (const auto& at : autotileOffsets)
    {
        std::array<int,4> charIndices;
        for (int i = 0; i < 4; i++)
            charIndices[i] = state.addCharacter(chars.at(at[i].x, at[i].y));
        state.tiles.push_back(std::move(charIndices));
    }

    return 0;
}

int processTileset(State& state, std::string name)
{
    util::grid<Character> chars;
    int error = ConvertPngToCharacters(chars, name);
    if (error)
    {
        std::cout << "Error while loading tileset " << name << ": ";
        std::cout << lodepng_error_text(error) << std::endl;
        return error;
    }

    if (chars.width() % 2 != 0 || chars.height() % 2 != 0)
    {
        std::cout << "Tileset " << name << " must have sizes multiple of 16!" << std::endl;
        return -1;
    }

    int w = chars.width()/2, h = chars.height()/2;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            std::array<int,4> charIndices;
            charIndices[0] = state.addCharacter(chars.at(2*i+0, 2*j+0));
            charIndices[1] = state.addCharacter(chars.at(2*i+1, 2*j+0));
            charIndices[2] = state.addCharacter(chars.at(2*i+0, 2*j+1));
            charIndices[3] = state.addCharacter(chars.at(2*i+1, 2*j+1));
            state.tiles.push_back(std::move(charIndices));
        }

    return 0;
}

Offset autotileOffsets[47][4] =
{
    // Corners
    { {2,4},{1,4},{2,3},{1,3} },
    { {2,0},{1,4},{2,3},{1,3} },
    { {2,4},{3,0},{2,3},{1,3} },
    { {2,0},{3,0},{2,3},{1,3} },
    { {2,4},{1,4},{2,3},{3,1} },
    { {2,0},{1,4},{2,3},{3,1} },
    { {2,4},{3,0},{2,3},{3,1} },
    { {2,0},{3,0},{2,3},{3,1} },
    { {2,4},{1,4},{2,1},{1,3} },
    { {2,0},{1,4},{2,1},{1,3} },
    { {2,4},{3,0},{2,1},{1,3} },
    { {2,0},{3,0},{2,1},{1,3} },
    { {2,4},{1,4},{2,1},{3,1} },
    { {2,0},{1,4},{2,1},{3,1} },
    { {2,4},{3,0},{2,1},{3,1} },
    { {2,0},{3,0},{2,1},{3,1} },

    // Sides
    { {0,4},{1,4},{0,3},{1,3} },
    { {0,4},{3,0},{0,3},{1,3} },
    { {0,4},{1,4},{0,3},{3,1} },
    { {0,4},{3,0},{0,3},{3,1} },
    { {2,2},{1,2},{2,3},{1,3} },
    { {2,2},{1,2},{2,3},{3,1} },
    { {2,2},{1,2},{2,1},{1,3} },
    { {2,2},{1,2},{2,1},{3,1} },
    { {2,4},{3,4},{2,3},{3,3} },
    { {2,4},{3,4},{2,1},{3,3} },
    { {2,0},{3,4},{2,3},{3,3} },
    { {2,0},{3,4},{2,1},{3,3} },
    { {2,4},{1,4},{2,5},{1,5} },
    { {2,0},{1,4},{2,5},{1,5} },
    { {2,4},{3,0},{2,5},{1,5} },
    { {2,0},{3,0},{2,5},{1,5} },

    // Pipes
    { {0,4},{3,4},{0,3},{3,3} },
    { {2,2},{1,2},{2,5},{1,5} },

    // Real corners
    { {0,2},{1,2},{0,3},{1,3} },
    { {0,2},{1,2},{0,3},{3,1} },
    { {2,2},{3,2},{2,3},{3,3} },
    { {2,2},{3,2},{2,1},{3,3} },
    { {2,4},{3,4},{2,5},{3,5} },
    { {2,0},{3,4},{2,5},{3,5} },
    { {0,4},{1,4},{0,5},{1,5} },
    { {0,4},{3,0},{0,5},{1,5} },

    // Dead-ends
    { {0,2},{3,2},{0,3},{3,3} },
    { {0,2},{1,2},{0,5},{1,5} },
    { {0,4},{3,4},{0,5},{3,5} },
    { {2,2},{3,2},{2,5},{3,5} },

    // Single block
    { {0,2},{3,2},{0,5},{3,5} }
};