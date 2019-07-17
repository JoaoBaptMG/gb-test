#include <iostream>
#include <fstream>
#include "char.hpp"
#include "nlohmann/json.hpp"

std::string pickTilesetName(const nlohmann::json& j);

struct RemappingInterval { int from, to, size; };

int addBlankRemappingInterval(const nlohmann::json& j, std::vector<RemappingInterval> &remappingInterval, int& curValue);
int addAutotileRemappingIntervals(const nlohmann::json& j, const std::string& at,
    std::vector<RemappingInterval> &remappingInterval, int& curValue);
int addTilesetRemappingInterval(const nlohmann::json& j, const std::string& tset,
    std::vector<RemappingInterval> &remappingInterval, int& curValue);
int getMapData(const nlohmann::json& j, std::vector<int>& data, int& width, int& height);

inline static nlohmann::json get(const nlohmann::json& j, const std::string& key)
{
    if (j.contains(key)) return j.at(key);
    return nlohmann::json();
}

int mapExport(int argc, char **argv)
{
    std::string in = argv[2];
    std::string out = argv[3];

    auto inlabel = labelizeName(in);

    auto fid = in.find_last_of('/');
    auto folder = fid == std::string::npos ? "" : in.substr(0, fid) + '/';

    nlohmann::json j;
    {
        std::ifstream inf(in);
        inf >> j;
    }

    int r;
    std::vector<int> data;
    int width, height;
    if ((r = getMapData(j, data, width, height))) return r;
    if (width < 10 || width > 255)
    {
        std::cout << "Error parsing map " << in << ": map must be at least 10 tiles and at most 255 tiles wide!" << std::endl;
        return -1;
    }
    if (height < 8 || height > 255)
    {
        std::cout << "Error parsing map " << in << ": map must be at least 8 tiles and at most 256 tiles high!" << std::endl;
        return -1;
    }

    std::string tilesetName = "";
    if (get(j, "properties").is_array())
    {
        for (const auto& obj : get(j, "properties"))
            if (get(obj, "name") == "tileset" && get(obj, "type") == "string" && get(obj, "value").is_string())
            {
                tilesetName = get(obj, "value").get<std::string>();
                break;
            }
    }
    if (tilesetName.empty())
    {
        std::cout << "Error parsing map " << in << ": provide a valid tileset name!" << std::endl;
        return -1;
    }

    nlohmann::json tj;
    {
        std::ifstream tinf(folder + "../tilesets/" + tilesetName);
        tinf >> tj;
    }

    auto at0 = get(tj, "autotile0").is_string() ? get(tj, "autotile0").get<std::string>() : "";
    auto at1 = get(tj, "autotile1").is_string() ? get(tj, "autotile1").get<std::string>() : "";
    auto tset = get(tj, "tileset").is_string() ? get(tj, "tileset").get<std::string>() : "";

    std::vector<RemappingInterval> remappingIntervals;
    int curValue = 0;
    if ((r = addBlankRemappingInterval(j, remappingIntervals, curValue))) return r;
    if (!at0.empty() && (r = addAutotileRemappingIntervals(j, at0, remappingIntervals, curValue))) return r;
    if (!at1.empty() && (r = addAutotileRemappingIntervals(j, at1, remappingIntervals, curValue))) return r;
    if (!tset.empty() && (r = addTilesetRemappingInterval(j, tset, remappingIntervals, curValue))) return r;

    for (int& d : data)
        for (const auto& in : remappingIntervals)
        {
            if (d >= in.from && d < in.from + in.size)
            {
                d += in.to - in.from;
                break;
            }
        }

    std::ofstream of(out);
    of << "; " << std::endl;
    of << "; " << out << std::endl;
    of << "; " << std::endl << std::endl;

    of << "section \"map metadata " << out <<  "\", rom0" << std::endl;
    of << inlabel << "::" << std::endl;
    of << "    db " << width << ", " << height << std::endl;
    of << "    db BANK(" << inlabel << "_data), LOW(" << inlabel << "_data), HIGH(" << inlabel << "_data)";
    of << std::endl << std::endl;

    of << "section \"map data " << out << "\", romx" << std::endl;
    of << inlabel << "_data:";
    for (std::size_t i = 0; i < data.size(); i++)
    {
        if (i % 16 == 0) of << std::endl << "    db ";
        else of << ", ";
        of << data[i];
    }
    of << std::endl;

    return 0;
}

int addBlankRemappingInterval(const nlohmann::json& j, std::vector<RemappingInterval> &remappingInterval, int& curValue)
{
    if (!get(j, "tilesets").is_array())
    {
        std::cout << "Error parsing map: failed to find tilesets property!" << std::endl;
        return -1;
    }

    remappingInterval.push_back({0, curValue, 1});

    for (const auto& ts : get(j, "tilesets"))
    {
        if (!get(ts, "image").is_string()) continue;
        auto image = get(ts, "image").get<std::string>();
        if (image.find("automappingRegions.png") == std::string::npos) continue;
        if (!get(ts, "firstgid").is_number_unsigned()) continue;
        int gid = get(ts, "firstgid").get<int>();
        remappingInterval.push_back({gid, curValue, 1});
        curValue++;

        return 0;
    }

    std::cout << "Error parsing map: failed to find blank tileset!" << std::endl;
    return -1;
}

int addAutotileRemappingIntervals(const nlohmann::json& j, const std::string& at,
    std::vector<RemappingInterval> &remappingInterval, int& curValue)
{
    auto v1 = at.find_last_of('/');
    auto v2 = at.find_last_of('.');
    auto atx = at.substr(v1, v2-v1);
    atx += ".tsx";

    if (!get(j, "tilesets").is_array())
    {
        std::cout << "Error parsing map: failed to find tilesets property!" << std::endl;
        return -1;
    }

    for (const auto& ts : get(j, "tilesets"))
    {
        if (!get(ts, "source").is_string()) continue;
        auto source = get(ts, "source").get<std::string>();
        if (source.find(atx) == std::string::npos) continue;
        if (!get(ts, "firstgid").is_number_unsigned()) continue;
        int gid = get(ts, "firstgid").get<int>();
        remappingInterval.push_back({gid, curValue, 47});
        remappingInterval.push_back({gid+47, curValue+46, 1});
        curValue += 47;

        return 0;
    }

    std::cout << "Error parsing map: failed to find tileset corresponding to autotile " << at << "!" << std::endl;
    return -1;
}

int addTilesetRemappingInterval(const nlohmann::json& j, const std::string& tset,
    std::vector<RemappingInterval> &remappingInterval, int& curValue)
{
    if (!get(j, "tilesets").is_array())
    {
        std::cout << "Error parsing map: failed to find tilesets property!" << std::endl;
        return -1;
    }

    for (const auto& ts : get(j, "tilesets"))
    {
        if (!get(ts, "image").is_string()) continue;
        auto image = get(ts, "image").get<std::string>();
        if (image.find(tset) == std::string::npos) continue;
        if (!get(ts, "firstgid").is_number_unsigned()) continue;
        int gid = get(ts, "firstgid").get<int>();
        if (!get(ts, "tilecount").is_number_unsigned()) continue;
        int tilecount = get(ts, "tilecount").get<int>();
        remappingInterval.push_back({gid, curValue, tilecount});
        curValue += tilecount;

        return 0;
    }

    std::cout << "Error parsing map: failed to find tileset corresponding to tileset " << tset << "!" << std::endl;
    return -1;
}

int getMapData(const nlohmann::json& j, std::vector<int>& data, int& width, int& height)
{
    if (!get(j, "layers").is_array())
    {
        std::cout << "Error parsing map: failed to find layers property!" << std::endl;
        return -1;
    }

    for (const auto& ly : get(j, "layers"))
    {
        if (get(ly, "name") != "map") continue;
        if (!get(ly, "width").is_number_unsigned()) continue;
        width = get(ly, "width").get<int>();
        if (!get(ly, "height").is_number_unsigned()) continue;
        height = get(ly, "height").get<int>();
        if (!get(ly, "data").is_array()) continue;
        data = get(ly, "data").get<std::vector<int>>();

        return 0;
    }

    std::cout << "Error parsing map: failed to find map data!" << std::endl;
    return -1;
}