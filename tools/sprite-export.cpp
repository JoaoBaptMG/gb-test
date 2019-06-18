#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include "nlohmann/json.hpp"
#include "lodepng.h"
#include "char.hpp"

int spriteExport(int argc, char **argv)
{
    std::string in = argv[2];
    std::string out = argv[3];

    util::grid<Character> chars;
    auto error = ConvertPngToCharacters(chars, in);
    if (error)
    {
        std::cerr << "Error converting image: " << lodepng_error_text(error) << std::endl;
        return -1;
    }

    std::ofstream of(out);
    of << "; " << std::endl;
    of << "; " << out << std::endl;
    of << "; " << std::endl << std::endl;

    bool banked = false;
    std::ifstream mdin(in + ".json");
    if (mdin.good())
    {
        nlohmann::json j;
        mdin >> j;
        banked = j.contains("banked") && j.at("banked").get<bool>();
    }

    of << "section \"sprite " << out << (banked ? "\", romx" : "\", rom0") << std::endl << std::endl; 

    auto inlabel = labelizeName(in);

    of << inlabel << "::" << std::endl;
    for (const auto& charac : chars)
        of << charac << std::endl;
    of << inlabel << "_end::" << std::endl;

    of.close();

    return 0;
}