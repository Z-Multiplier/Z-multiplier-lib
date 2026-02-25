//MIT License

//Copyright (c) 2026 Z-Multiplier
#include "GameObject.hpp"
#include "DefaultAIBehavior.hpp"
#include "Utils.hpp"
#include <algorithm>
Game::AIBehavior Game::Defaults::wander(float speed,int range,float frequency){
    Game::AIBehavior beh;
    beh.addCondition([frequency](int,int,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&)
    {
        Utils::Random random;
        float r=random.real(0,1);
        return r<frequency;
    });
    beh.setAction([speed,range](int& x,int& y,std::unordered_map<std::string,std::variant<int,float,bool,std::string>>& attributes,std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>& listAttributes){
        Utils::Random random;
        int targetX=x+random.range(-range,range);
        int targetY=y+random.range(-range,range);
        auto path=AStar(std::make_pair(x,y),std::make_pair(targetX,targetY),Defaults::defaultDeltas,Defaults::defaultExtraChecks,
                        [](std::pair<int,int> s,std::pair<int,int> e){return std::sqrt(std::pow(s.first-e.first,2)+std::pow(s.second-e.second,2));});
        std::vector<int> xs;
        std::vector<int> ys;
        xs.reserve(path.size());
        ys.reserve(path.size());
        for(const auto& p:path){
            xs.push_back(p.first);
            ys.push_back(p.second);
        }
        std::vector<std::variant<int,float,bool,std::string>> vxs;
        std::vector<std::variant<int,float,bool,std::string>> vys;
        vxs.reserve(xs.size());
        vys.reserve(ys.size());
        for(const auto& x:xs){
            vxs.push_back(x);
        }
        for(const auto& y:ys){
            vys.push_back(y);
        }
        listAttributes["pathXs"]=std::move(vxs);
        listAttributes["pathYs"]=std::move(vys);
        attributes["speed"]=speed;
        return;
    });
    return beh;
}
Game::AIBehavior Game::Defaults::updateAttributes(){
    Game::AIBehavior beh;
    beh.addCondition([](int,int,const Game::attrMap&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>&){return true;});
    beh.setAction([](int& x,int& y,
                    Game::attrMap& attributes,
                    auto& listAttributes){
        auto itXs=listAttributes.find("pathXs");
        auto itYs=listAttributes.find("pathYs");
        if(itXs==listAttributes.end()||itYs==listAttributes.end()){
            attributes["moving"]=false;
            return;
        }
        const auto& xs=itXs->second;
        const auto& ys=itYs->second;
        if(xs.empty()||ys.empty()||xs.size()!=ys.size()){
            attributes["moving"]=false;
            return;
        }
        int pathIndex=0;
        auto itIndex=attributes.find("pathIndex");
        if(itIndex!=attributes.end()&&std::holds_alternative<int>(itIndex->second)){
            pathIndex=std::get<int>(itIndex->second);
        }
        else{
            attributes["pathIndex"]=pathIndex;
        }
        if(pathIndex>=static_cast<int>(xs.size())){
            listAttributes.erase("pathXs");
            listAttributes.erase("pathYs");
            attributes.erase("pathIndex");
            attributes["moving"]=false;
            return;
        }
        int targetX=std::get<int>(xs[pathIndex]);
        int targetY=std::get<int>(ys[pathIndex]);
        float px=0.0f,py=0.0f;
        auto itPx=attributes.find("px");
        auto itPy=attributes.find("py");
        if(itPx!=attributes.end()&&std::holds_alternative<float>(itPx->second))
            px=std::get<float>(itPx->second);
        if(itPy!=attributes.end()&&std::holds_alternative<float>(itPy->second))
            py=std::get<float>(itPy->second);
        float speed=0.0f;
        auto itSpeed=attributes.find("speed");
        if(itSpeed!=attributes.end()&&std::holds_alternative<float>(itSpeed->second)){
            speed=std::get<float>(itSpeed->second);
        }
        else{
            attributes["moving"]=false;
            return;
        }
        float targetPx=targetX*Game::nowTerrain->gridWSize+Game::nowTerrain->gridWSize/2.0f;
        float targetPy=targetY*Game::nowTerrain->gridHSize+Game::nowTerrain->gridHSize/2.0f;
        float dx=targetPx-px;
        float dy=targetPy-py;
        float distance=std::sqrt(dx*dx+dy*dy);
        if(distance<=speed){
            px=targetPx;
            py=targetPy;
            pathIndex++;
            attributes["pathIndex"]=pathIndex;
        }
        else{
            float ratio=speed/distance;
            px += dx*ratio;
            py += dy*ratio;
        }
        attributes["px"]=px;
        attributes["py"]=py;
        int newGridX=static_cast<int>(px/Game::nowTerrain->gridWSize);
        int newGridY=static_cast<int>(py/Game::nowTerrain->gridHSize);
        newGridX=std::clamp(newGridX,0,Game::nowTerrain->height-1);
        newGridY=std::clamp(newGridY,0,Game::nowTerrain->width-1);
        x=newGridX;
        y=newGridY;
        attributes["x"]=newGridX;
        attributes["y"]=newGridY;
        if(distance>0){
            attributes["facing"]=std::atan2(dy,dx);
        }
        if(pathIndex>=static_cast<int>(xs.size())){
            listAttributes.erase("pathXs");
            listAttributes.erase("pathYs");
            attributes.erase("pathIndex");
            attributes["moving"]=false;
        }
        else{
            attributes["moving"]=true;
        }
    });
    return beh;
}