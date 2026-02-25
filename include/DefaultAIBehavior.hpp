//MIT License

//Copyright (c) 2026 Z-Multiplier
#ifndef DEFAULTAIBEHAVIOR_HPP
#define DEFAULTAIBEHAVIOR_HPP
#include "GameObject.hpp"
namespace Game{
    using attrMap=std::unordered_map<std::string,std::variant<int,float,bool,std::string>>;
    namespace Defaults{
        AIBehavior updateAttributes();
        AIBehavior wander(float speed,int range,float frequency);
        //AIBehavior flee(Life* source,float speed,int range);
        //AIBehavior rage(Life* target,float speed,int until);
    }
}

#endif