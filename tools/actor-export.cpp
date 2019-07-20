#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"
#include "actor-export.hpp"

std::map<std::string,ActorType> parseActorTypes(std::string file)
{
    nlohmann::json j;
    {
        std::ifstream inf(file);
        inf >> j;
    }

    std::map<std::string,ActorType> actorTypes;

    for (const auto& item : j.items())
    {
        if (!item.value().is_array()) continue;

        ActorType actor;

        for (const auto& param : item.value())
        {
            Parameter parameter;

            parameter.offset = param.at("offset").get<std::size_t>();
            parameter.size = param.at("size").get<std::size_t>();

            if (param.contains("param"))
            {
                parameter.parameter = param.at("param").get<std::string>();
                if (param.contains("mult"))
                    parameter.valueOrMult = param.at("mult").get<std::intmax_t>();
                else parameter.valueOrMult = 1;
            }
            else if (param.contains("value"))
                parameter.valueOrMult = param.at("value").get<std::intmax_t>();
            else parameter.valueOrMult = 0;

            actor.parameters.push_back(std::move(parameter));
        }

        actorTypes.emplace(item.key(), std::move(actor));
    }

    int id = 1;
    for (auto& actor : actorTypes)
        actor.second.type = id++;

    return actorTypes;
}

int actorExport(int argc, char **argv)
{
    std::string in = argv[2];
    std::string out = argv[3];

    nlohmann::json j;
    {
        std::ifstream inf(in);
        inf >> j;
    }

    std::vector<std::string> actorTypes{"NoActor"};
    for (const auto& item : j.items())
        actorTypes.push_back(item.key());

    std::sort(actorTypes.begin()+1, actorTypes.end());

    if (actorTypes.size() > 256)
    {
        std::cout << "There can be a maximum of 255 actor types!" << std::endl;
        return -1;
    }

    actorTypes.resize(256, "NoActor");

    std::ofstream of(out);
    of << "; " << std::endl;
    of << "; " << out << std::endl;
    of << "; " << std::endl << std::endl;

    of << "section \"rom " << out << "\", rom0, align[8]" << std::endl;
    of << "ActorTypeTable::";

    for (int i = 0; i < 3; i++)
    {
        int k = 0;
        for (const auto& ac : actorTypes)
        {
            if (k % 4 == 0) of << std::endl << "    db ";
            else of << ", ";
            k++;

            switch (i)
            {
                case 0: of << "BANK(" << ac << "_Update)"; break;
                case 1: of << "LOW(" << ac << "_Update)"; break;
                case 2: of << "HIGH(" << ac << "_Update)"; break;
            }
        }

        of << std::endl;
    }

    of << std::endl << "section \"gtable " << out << "\", rom0, align[9]" << std::endl;
    of << "ActorTypeGraphicsTable::";
    int k = 0;
    for (const auto& ac : actorTypes)
    {
        if (k % 4 == 0) of << std::endl << "    dw ";
        else of << ", ";
        k++;

        of << ac << "_GraphicsTable";
    }

    of << std::endl << std::endl << "section \"wram " << out << "\", wram0, align[8]" << std::endl;
    of << "wTypeGraphics::" << std::endl;
    bool noActorEnd = false;
    for (const auto& ac : actorTypes)
    {
        if (ac == "NoActor")
        {
            if (noActorEnd) break;
            else of << "    db" << std::endl;
            noActorEnd = true;
        }
        else of << "wSlot_" << ac << ":: db" << std::endl;
    }

    of << std::endl;

    return 0;
}