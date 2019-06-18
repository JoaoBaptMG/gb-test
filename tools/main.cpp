#include <iostream>
#include <string>
#include <map>
#include "lodepng.h"

using Tool = int(*)(int argc, char **argv);

int spriteExport(int argc, char **argv);
int tilemapExport(int argc, char **argv);
int mapExport(int argc, char **argv);

const std::map<std::string, Tool> toolList =
{
    { "sprite-export", spriteExport },
    { "tileset-export", tilemapExport },
    { "map-export", mapExport },
};

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " <tool> <input> <output>" << std::endl;
        return -1;
    }

    auto it = toolList.find(argv[1]);
    if (it == toolList.end())
    {
        std::cout << "Tool " << argv[1] << " not found in the tool list!";
        return -1;
    }

    return it->second(argc, argv);
}