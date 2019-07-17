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
    bool vertical = false;
    std::size_t groupWidth = 1;
    std::size_t groupHeight = 1;
    std::ifstream mdin(in + ".json");
    if (mdin.good())
    {
        nlohmann::json j;
        mdin >> j;
        banked = j.contains("banked") && j.at("banked").get<bool>();
        vertical = j.contains("vertical") && j.at("vertical").get<bool>();
        if (j.contains("group_width") && j.at("group_width").is_number_unsigned())
            groupWidth = j.at("group_width").get<std::size_t>();
        if (j.contains("group_height") && j.at("group_height").is_number_unsigned())
            groupHeight = j.at("group_height").get<std::size_t>();
    }

    of << "section \"sprite " << out << (banked ? "\", romx" : "\", rom0") << std::endl << std::endl; 

    auto inlabel = labelizeName(in);

    of << inlabel << "::" << std::endl;
    for (std::size_t j = 0; j < chars.height(); j += groupHeight)
        for (std::size_t i = 0; i < chars.width(); i += groupWidth)
        {
            std::size_t cellWidth = std::min(chars.width() - i, groupWidth);
            std::size_t cellHeight = std::min(chars.height() - j, groupHeight);
            for (const auto& charac : chars.make_view(i, j, cellWidth, cellHeight))
                of << charac << std::endl;
        }
    of << inlabel << "_end::" << std::endl;

    of.close();

    return 0;
}