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
        ActorType actor;
        actor.paramSize = 0;

        for (const auto& param : item.value().items())
        {
            auto arr = param.value().get<std::vector<std::size_t>>();
            actor.parameters.emplace(param.key(), Parameter{ arr[0], arr[1] });
            auto maxParamSize = arr[0] + arr[1];
            if (actor.paramSize < maxParamSize)
                actor.paramSize = maxParamSize;
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

    for (int i = 0; i < 5; i++)
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
                case 3: of << "LOW(" << ac << "_Init)"; break;
                case 4: of << "HIGH(" << ac << "_Init)"; break;
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