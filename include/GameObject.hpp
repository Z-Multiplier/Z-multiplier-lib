//MIT License

//Copyright (c) 2026 Z-Multiplier
#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP
#include <unordered_map>
#include <variant>
#include <string>
#include <functional>
#include <vector>
#include <optional>
#include <limits>
#include "Image.hpp"
namespace Game{
    struct AIBehavior{
        std::vector<std::function<bool(int,int,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)>> checkers;
        std::function<void(int&,int&,std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)> action;
        bool valid(int x,int y,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)const;
        void addCondition(std::function<bool(int,int,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)> checker);
        void setAction(std::function<void(int&,int&,std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)> act);
    };
    struct Life{
        std::unordered_map<std::string,std::variant<int,float,bool,std::string>> attributes;
        std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>> listAttributes;
        std::vector<AIBehavior> behaviors;
        void act();
        std::variant<int,float,bool,std::string>* operator[](const std::string&);
        std::vector<std::variant<int,float,bool,std::string>>* list(const std::string&);
        std::variant<int,float,bool,std::string>* value(const std::string&);
        Life(){
            attributes["x"]=0;
            attributes["y"]=0;
            attributes["px"]=0.0f;
            attributes["py"]=0.0f;
            attributes["facing"]=0.0f;
        }
    };
    namespace Defaults{
        inline const std::vector<std::string> defaultGridTypes={
            "NULL",
            "EMPTY",
            "UNWALKABLE",
            "WALKABLE",
            "WALKABLE_LOW_COST",
            "WALKABLE_HIGH_COST",
            "WALKABLE_NO_COST"
        };
        inline const std::unordered_map<std::string,float> defaultCostRules={
            {"NULL",std::numeric_limits<float>::infinity()},
            {"EMPTY",std::numeric_limits<float>::infinity()},
            {"UNWALKABLE",std::numeric_limits<float>::infinity()},
            {"WALKABLE",0.1f},
            {"WALKABLE_LOW_COST",0.5f},
            {"WALKABLE_HIGH_COST",1.1f},
            {"WALKABLE_NO_COST",0.0f}
        };
        inline const std::vector<std::pair<std::string,std::wstring>> defaultTexturePaths={
            {"NULL",L"./null.png"},
            {"EMPTY",L"./empty.png"},
            {"UNWALKABLE",L"./unwalkable.png"},
            {"WALKABLE",L"./walkable.png"},
            {"WALKABLE_LOW_COST",L"./walkable_low_cost.png"},
            {"WALKABLE_HIGH_COST",L"./walkable_high_cost.png"},
            {"WALKABLE_NO_COST",L"./walkable_no_cost.png"}
        };
        inline std::vector<std::vector<std::string>> terrainAllWalkable(int h,int w){
            return std::vector<std::vector<std::string>>(h,std::vector<std::string>(w,"WALKABLE"));
        }
        inline std::vector<std::vector<std::string>> terrainAllNull(int h,int w){
            return std::vector<std::vector<std::string>>(h,std::vector<std::string>(w,"NULL"));
        }
        inline std::vector<std::pair<int,int>> defaultDeltas={
            {1,0},
            {-1,0},
            {0,1},
            {0,-1},
            {1,1},
            {1,-1},
            {-1,-1},
            {-1,1},
            {2,1},
            {-2,1},
            {-2,-1},
            {2,-1},
            {1,2},
            {1,-2},
            {-1,-2},
            {-1,2}
        };
        inline std::vector<std::vector<std::pair<int,int>>> defaultExtraChecks={
            {},
            {},
            {},
            {},
            {{0,1},{1,0}},
            {{0,-1},{1,0}},
            {{0,-1},{-1,0}},
            {{0,1},{-1,0}},
            {{1,0},{1,1}},
            {{-1,0},{-1,1}},
            {{-1,0},{-1,-1}},
            {{1,0},{1,-1}},
            {{0,1},{1,1}},
            {{0,1},{1,-1}},
            {{0,-1},{-1,-1}},
            {{0,1},{-1,1}}
        };
    }
    struct Terrain{
        int height,width;
        int gridWSize,gridHSize;
        std::unordered_map<std::string,Assets::Image> gridTextures;
        std::vector<std::vector<float>> cost;
        std::vector<std::string> allTypes;
        std::vector<std::vector<std::string>> gridtypes;
        Terrain()=delete;
        Terrain(int windowH,int windowW,int thisH,int thisW,std::vector<std::string> gridTypes,std::vector<std::pair<std::string,std::wstring>> textures,std::vector<std::vector<std::string>> terrain,std::unordered_map<std::string,float> rules);
    };
    inline Terrain *nowTerrain;
    std::vector<std::pair<int,int>> AStar(std::pair<int,int> start,std::pair<int,int> goal,const std::vector<std::pair<int,int>>& deltas,const std::vector<std::vector<std::pair<int,int>>>& extraChecks,std::function<float(std::pair<int,int>,std::pair<int,int>)> guess,float weight=1.5f);
}

#endif