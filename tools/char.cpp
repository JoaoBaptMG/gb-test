#include <iostream>

#include "char.hpp"
#include "lodepng.h"

unsigned int ConvertPngToCharacters(util::grid<Character>& grid, std::string filename)
{
    std::vector<std::uint8_t> image;
    unsigned int width, height;
    unsigned int error = lodepng::decode(image, width, height, filename, LCT_PALETTE, 8U);
    if (error) return error;

    if (width % 8 != 0 || height % 8 != 0)
    {
        std::cout << "Warning! PNG image is not multiple of 8!";
        return -1;
    }

    int tw = width/8, th = height/8;
    util::grid<Character> output(tw, th);

    for (int j = 0; j < th; j++)
        for (int i = 0; i < tw; i++)
            for (int r = 0; r < 8; r++)
            {
                std::uint8_t char1 = 0;
                std::uint8_t char2 = 0;

                for (int c = 0; c < 8; c++)
                {
                    int color = image.at((j*8 + r)*width + (i*8 + c));
                    char1 = (char1 << 1) | (color & 1);
                    char2 = (char2 << 1) | ((color >> 1) & 1);
                }

                output.at(i, j).at(2*r) = char1;
                output.at(i, j).at(2*r+1) = char2;
            }

    grid = std::move(output);
    return 0;
}

std::ostream& operator<<(std::ostream& out, const Character& charac)
{
    bool nf = false;
    for (std::uint8_t b : charac)
    {
        if (nf) out << ", ";
        else out << "    db ";
        out << (std::size_t)b;
        nf = true;
    }
    return out;
}

std::string labelizeName(std::string name)
{
    for (auto& c : name)
        if (!std::isalnum(c)) c = '_';
    return name;
}