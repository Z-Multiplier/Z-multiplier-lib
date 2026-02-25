//MIT License

//Copyright (c) 2026 Z-Multiplier
#include "GameObject.hpp"
#include <unordered_map>
#include <map>
#include <variant>
#include <string>
#include <functional>
#include <vector>
#include <queue>
#include <optional>
#include <algorithm>
#include "Logger.hpp"
Core::logger GameObjectLogger;
bool Game::AIBehavior::valid(int x,int y,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>& attribute,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>& listAttributes)const{
    for(const auto& checker:checkers){
        if(checker(x,y,attribute,listAttributes)==false){
            return false;
        }
    }
    return true;
}
void Game::AIBehavior::addCondition(std::function<bool(int,int,const std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,const std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>& listAttributes)> checker){
    checkers.push_back(checker);
}
void Game::AIBehavior::setAction(std::function<void(int&,int&,std::unordered_map<std::string,std::variant<int,float,bool,std::string>>&,std::unordered_map<std::string,std::vector<std::variant<int,float,bool,std::string>>>& listAttributes)> act){
    action=act;
}
void Game::Life::act(){
    for(const auto& beh:behaviors){
        if(beh.valid(std::get<int>(attributes["x"]),std::get<int>(attributes["y"]),attributes,listAttributes)){
            beh.action(std::get<int>(attributes["x"]),std::get<int>(attributes["y"]),attributes,listAttributes);
            return;
        }
    }
}
std::variant<int,float,bool,std::string>* Game::Life::operator[](const std::string& key){
    if(attributes.count(key)) return &attributes.at(key);
    return nullptr;
}
std::vector<std::variant<int,float,bool,std::string>>* Game::Life::list(const std::string& key){
    if(listAttributes.count(key)) return &listAttributes.at(key);
    return nullptr;
}
std::variant<int,float,bool,std::string>* Game::Life::value(const std::string& key){
    if(attributes.count(key)) return &attributes.at(key);
    return nullptr;
}
Game::Terrain::Terrain(int windowH,int windowW,int thisH,int thisW,std::vector<std::string> gridTypes,std::vector<std::pair<std::string,std::wstring>> textures,std::vector<std::vector<std::string>> terrain,std::unordered_map<std::string,float> rules){
    this->height=thisH;
    this->width=thisW;
    this->gridtypes=terrain;
    this->allTypes=gridTypes;
    for(const auto& texture:textures){
        this->gridTextures[texture.first]=Assets::Image(texture.second);
    }
    cost.resize(thisH);
    for(int i=0;i<thisH;i++){
        cost[i].resize(thisW);
        for(int j=0;j<thisW;j++){
            cost[i][j]=rules[terrain[i][j]];
        }
    }
    if(windowH%thisH!=0||windowW%thisW!=0){
        GameObjectLogger.traceLog(Core::logger::LOG_ERROR,"The gridsize is not an integer");
        throw std::runtime_error("Error:The gridsize is not an integer");
    }
    int aw=windowW/thisW;
    int ah=windowH/thisH;
    this->gridHSize=ah;
    this->gridWSize=aw;
}
std::vector<std::pair<int,int>> Game::AStar(std::pair<int,int> start,std::pair<int,int> goal,const std::vector<std::pair<int,int>>& deltas,const std::vector<std::vector<std::pair<int,int>>>& extraChecks,std::function<float(std::pair<int,int>,std::pair<int,int>)> guess,float weight){
    if(deltas.size()!=extraChecks.size()){
        GameObjectLogger.traceLog(Core::logger::LOG_ERROR,"The size of the delta is not equal to the size of checks");
        throw std::runtime_error("Error:The size of the delta is not equal to the size of checks");
    }
    std::vector<std::pair<int,int>> path;
    std::map<std::pair<int,int>,float> g;
    std::map<std::pair<int,int>,std::pair<int,int>> from;
    std::priority_queue<std::pair<float,std::pair<int,int>>,std::vector<std::pair<float,std::pair<int,int>>>,std::greater<std::pair<float,std::pair<int,int>>>> open;
    std::map<std::pair<int,int>,bool> closed;
    bool found=false;
    g[start]=0.0f;
    float hStart=weight*guess(start,goal);
    open.push(make_pair(hStart,start));
    while(!open.empty()){
        auto nowPoint=open.top().second;
        if(nowPoint==goal) found=true;
        if(found) break;
        open.pop();
        if(closed[nowPoint]) continue;
        closed[nowPoint]=true;
        for(unsigned int idx=0;idx<deltas.size();idx++){
            int nx=nowPoint.first+deltas[idx].first;
            int ny=nowPoint.second+deltas[idx].second;
            if(nx<0||nx>=Game::nowTerrain->height||ny<0||ny>=Game::nowTerrain->width) continue;
            auto nextPoint=std::make_pair(nx,ny);
            float moveCost=std::sqrt(deltas[idx].first*deltas[idx].first+deltas[idx].second*deltas[idx].second)+Game::nowTerrain->cost[nowPoint.first][nowPoint.second]+Game::nowTerrain->cost[nx][ny];
            for(const auto& check:extraChecks[idx]){
                int cx=nowPoint.first+check.first;
                int cy=nowPoint.second+check.second;
                if(cx>=0&&cx<Game::nowTerrain->height&&cy>=0&&cy<Game::nowTerrain->width){
                    moveCost+=Game::nowTerrain->cost[cx][cy];
                }
            }
            float tentativeG=g[nowPoint]+moveCost;
            if(!g.count(nextPoint)||tentativeG<g[nextPoint]){
                g[nextPoint]=tentativeG;
                float h=weight*guess(nextPoint,goal);
                float f=tentativeG+h;
                from[nextPoint]=nowPoint;
                open.push({f,nextPoint});
                if(closed.count(nextPoint)&&closed[nextPoint]){
                    closed[nextPoint]=false;
                }
            }
            /*auto nextOpen=std::make_pair(sqrt(deltas[idx].first*deltas[idx].first+deltas[idx].second*deltas[idx].second)+Game::nowTerrain->cost[nowPoint.first][nowPoint.second]+Game::nowTerrain->cost[nowPoint.first+deltas[idx].first][nowPoint.second+deltas[idx].second],std::make_pair(nowPoint.first+deltas[idx].first,nowPoint.second+deltas[idx].second));
            for(const auto& check:extraChecks[idx]){
                nextOpen.first+=Game::nowTerrain->cost[check.first][check.second];
            }
            nextOpen.first+=guess(nowPoint,std::make_pair(nowPoint.first+deltas[idx].first,nowPoint.second+deltas[idx].second));
            from[nextOpen.second]=nowPoint;
            if(nextOpen.second==goal) found=true;
            if(found) break;
            open.push(nextOpen);*/
        }
    }
    if(!found) return path;
    for(auto point=goal;point!=start;point=from[point]){
        path.push_back(point);
    }
    std::reverse(path.begin(),path.end());
    return path;
}