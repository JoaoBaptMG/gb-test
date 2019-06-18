#include <cstdint>
#include <ostream>
#include <array>
#include "grid.hpp"
#include "lodepng.h"

#pragma once

constexpr std::size_t CharSize = 16;
using Character = std::array<std::uint8_t, CharSize>;

unsigned int ConvertPngToCharacters(util::grid<Character>& grid, std::string filename);
std::string labelizeName(std::string name);

std::ostream& operator<<(std::ostream& out, const Character& tile);
