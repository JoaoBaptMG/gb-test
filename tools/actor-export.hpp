#pragma once

#include <map>

struct Parameter { std::size_t ofs, size; };

struct ActorType
{
    std::size_t type;
    std::size_t paramSize;
    std::map<std::string,Parameter> parameters;
};

std::map<std::string,ActorType> parseActorTypes(std::string file);
