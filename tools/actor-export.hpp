#pragma once

#include <vector>
#include <string>
#include <map>

struct Parameter
{ 
    std::size_t offset, size;
    std::string parameter;
    std::intmax_t valueOrMult;
};

struct ActorType
{
    std::size_t type;
    std::vector<Parameter> parameters;
};

std::map<std::string,ActorType> parseActorTypes(std::string file);
