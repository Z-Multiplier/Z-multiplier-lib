#include "Graphics.hpp"
#include "Logger.hpp"
#include <chrono>
#include <map>
#include <algorithm>
#include <random>
using namespace Graphics;

long double arc=0.0f;//方向
int pX=200,pY=200;//坐标
int mX=0,mY=0;//鼠标坐标
int nowSpeed=0;//当前速度
int enemySpawnCooldown=400;//敌机生成冷却
double currentDifficulty=0.1f;//现在的难度
int playerHP=20;//玩家血量
bool waitingForQuit=false;//死亡标记
int shootCooldown=120;//射击冷却
int nowexp=19;//现有经验值（19是我debug用的）
bool canChooseSkill=false;//废弃变量
int level=0;//等级
int reactDuration=0;//应激时长
float totalRage=0.0f;//总仇恨
float difficulty=0.0f;//别问，问就是隔一天写忘了自己定义了什么了
//罗马数字转换（支持 1~5）
std::wstring toRoman(int num){
    const std::pair<int,const wchar_t*> values[]={
    {5,L"V"},{4,L"IV"},{1,L"I"}
    };
    std::wstring result;
    for(auto& v:values){
        while(num>=v.first){
            result+=v.second;
            num-=v.first;
        }
    }
    return result;
}
struct bullet{
    int x,y;
    long double facing;//in arc obviously!
    bullet()=delete;
    bullet(int x,int y,long double facing):x(x),y(y),facing(facing){};
};
struct enemybullet{
    int x,y;
    long double facing;//in arc obviously!
    enemybullet()=delete;
    enemybullet(int x,int y,long double facing):x(x),y(y),facing(facing){};
};//Ctrl+C Ctrl+V大佬
struct enemy{
    int x,y;
    long double facing;
    int shootCooldown;
    int HP=20;
};
struct wingman{
    int x,y;
    long double facing;
    int shootCooldown;
    wingman()=default;
    wingman(int x,int y):x(x),y(y),facing(0.0),shootCooldown(0){}
};
std::vector<wingman> wingmans;
struct EXPorb{
    int x;
    int y;
    int exp;
};
struct Skill{
    std::wstring name;
    int maxLevel;
    std::wstring description;
    Assets::Image icon;
    float difficultyInfluence;
    Skill(const Skill&)=delete;
    Skill& operator=(const Skill&)=delete;
    Skill(Skill&&) noexcept=default;
    Skill& operator=(Skill&&) noexcept=default;
    Skill(std::wstring n,std::wstring d,int ml,std::wstring path,float di):name(n),maxLevel(ml),description(d),icon(path),difficultyInfluence(di){}
};//技能
vector<Skill> makeSkillPool(){
    vector<Skill> v;
    v.emplace_back(L"攻速突破",L"无视上限，提升 1/2/3/4/5 frame攻速",5,L"./atkspd.bmp",0.1);
    v.emplace_back(L"加固护甲",L"提升 5/10/15 最大生命值",3,L"./hpup.bmp",0.1);
    v.emplace_back(L"推进引擎",L"提升 ~1/2/3/4 移动速度",4,L"./speed.bmp",0.1);
    v.emplace_back(L"增援",L"召唤 1/2/3/4/5 架僚机， V级时可以右键长按召回僚机",5,L"./extraplane.bmp",0.2);
    v.emplace_back(L"多重射击",L"每级向两侧分别增加 1/2 枚子弹",2,L"./multishot.bmp",0.2);
    v.emplace_back(L"美食家",L"允许你吃掉敌机，回复 1/2/3/4/5 点生命值",5,L"./eater.bmp",0.1);
    v.emplace_back(L"应急能源",L"一次性，濒死时复活并带有 1 点生命值",1,L"./rebirth.bmp",0.1);
    v.emplace_back(L"学习",L"受伤时概率获得 1/2/3 经验",3,L"./learn.bmp",0.1);
    v.emplace_back(L"多向炮台",L"长按左键，向 4/8个基本方向射出子弹",2,L"./eightshoot.bmp",0.2);
    v.emplace_back(L"隧穿",L"失去右键移动能力，但改为 向鼠标方向瞬移 60/120 px",2,L"./tp.bmp",0.2);
    v.emplace_back(L"应激",L"受伤后提升 ~2/4 移动速度，持续 60 frame",2,L"./react.bmp",0.1);
    v.emplace_back(L"拾荒",L"捡起经验球时回复 1/1/2 生命值",3,L"./recycle.bmp",0.1);
    v.emplace_back(L"闪电链",L"子弹命中后链接 1/2/3 个敌人，每击中一个敌人，衰减 30%",3,L"./electricchain.bmp",0.3);
    v.emplace_back(L"沉重核心",L"降低 ~3 移动速度",1,L"./heavycore.bmp",-0.3);
    return v;
}
vector<Skill> skillPool=makeSkillPool();
std::unordered_map<std::wstring,int> learnedSkills;//name,level

//helper: wstring -> utf8 string for logging
static std::string toUTF8(const std::wstring &w){
    if(w.empty()) return std::string();
    int size_needed=WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),NULL,0,NULL,NULL);
    std::string s(size_needed,0);
    WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),&s[0],size_needed,NULL,NULL);
    return s;
}

//升级弹窗相关状态
bool selectingSkill=false;
std::vector<int> skillChoices; //当前弹窗显示的技能在skillPool中的下标
int selectedSkill=-1; //玩家选择的技能下标
vector<bullet> bullets;
vector<std::pair<Point,unsigned char>> stars;//location and shininess
vector<enemy> enemies;
vector<enemybullet> enemybullets;
vector<EXPorb> exporbs;
vector<HWND> skillWindowHandles;
Core::logger GameLogger;
void updateWingmans(){
    int targetCount=learnedSkills.count(L"增援")?learnedSkills[L"增援"]:0;
    if(wingmans.size()==targetCount) return;
    //增加僚机至目标数量
    while(wingmans.size()<targetCount){
        int offsetX=(rand() % 100)-50;
        int offsetY=(rand() % 100)-50;
        wingmans.emplace_back(pX+offsetX,pY+offsetY);
    }
    //减少多余僚机（技能等级不会降低，但保留处理）
    while(wingmans.size()>targetCount){
        wingmans.pop_back();
    }
}
long long mainWindowPaint(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,Painter& painter){//参数是锁死的，只能写这些
    static auto __start_time=std::chrono::steady_clock::now();
    auto __now_time=std::chrono::steady_clock::now();
    double elapsed=std::chrono::duration<double>(__now_time-__start_time).count();//用于计算难度
    //update difficulty D
    double D=0.1+0.001*elapsed+0.2*level+totalRage;//计算公式
    //别问公式哪来的，问就是试出来的
    currentDifficulty=D;
    if(shootCooldown>0){
        shootCooldown--;
    }//射击冷却，这个应该是比较容易想到的思路
    //666还有错别字
    painter.drawBackground(Color((unsigned char)0,0,0,255));//background，字面意思
    if(shootCooldown)//冷却，画红线，OIer特有的能省略就省略
        painter.line(Point(pX,pY),Point(mX,mY),Color((unsigned char)255,0,0,255));
    else//非冷却，画黄线
        painter.line(Point(pX,pY),Point(mX,mY),Color((unsigned char)255,255,0,255));
    //我爱死输入法了:P
    painter.solidCircle(Point(mX,mY),5,Color((unsigned char)0,255,0,255));//hint line，光标部分
    for(auto& star:stars){
        painter.putPixel(star.first.x,star.first.y,Color((unsigned char)255,255,255,++star.second));//让星星慢慢变亮
        if(star.second==255){//最亮时换位置，模拟新出现的星星
            star.second=0;
            star.first.x=rand()%700;
            star.first.y=rand()%700;//别问，问就是魔法数字
        }
    }
    static Assets::Image img;//玩家贴图
    static Assets::Image bulletTexture;//子弹贴图
    static Assets::Image enemyBulletTexture;//敌人子弹贴图
    img=Assets::Image(L"./www.bmp");//文件名
    bulletTexture=Assets::Image(L"./bullet.bmp");
    enemyBulletTexture=Assets::Image(L"./enemybullet.bmp");
    Assets::Matrix m(arc);//根据弧度获取旋转矩阵
    img.matrix=m;//存入图片矩阵
    img.transformation(false);//变换
    painter.putImage(LOCATEMODE_CENTER,Point(pX,pY),img,255);//绘制玩家
    vector<Point> healthBar={{pX-playerHP*2,pY-30},{pX-playerHP*2,pY-35},
                            {pX+playerHP*2,pY-35},{pX+playerHP*2,pY-30}};//生命条
    painter.solidPolygon(healthBar,Color((unsigned char)0,255,0,127));//绘制
    vector<Point> expBar={{pX-nowexp*2,pY-40},{pX-nowexp*2,pY-42},
                            {pX+nowexp*2,pY-42},{pX+nowexp*2,pY-40}};//经验条
    painter.solidPolygon(expBar,Color((unsigned char)255,255,0,127));//同理
    for(int idx=0;idx<bullets.size();idx++){//遍历所有子弹
        bullet& nbullet=bullets[idx];
        painter.putImage(LOCATEMODE_CENTER,Point(nbullet.x,nbullet.y),bulletTexture,255);
        nbullet.x+=7*cos(nbullet.facing);
        nbullet.y+=7*sin(nbullet.facing);//三角函数计算位移
        if(nbullet.x<0||nbullet.x>700||nbullet.y<0||nbullet.y>700){
            bullets.erase(bullets.begin()+idx);
            idx--;
        }
    }
    for(int idx=0;idx<enemybullets.size();idx++){//对敌人的子弹同理
        enemybullet& nebullet=enemybullets[idx];
        painter.putImage(LOCATEMODE_CENTER,Point(nebullet.x,nebullet.y),enemyBulletTexture,255);
        nebullet.x+=7*cos(nebullet.facing);
        nebullet.y+=7*sin(nebullet.facing);
        if(nebullet.x<0||nebullet.x>700||nebullet.y<0||nebullet.y>700){
            enemybullets.erase(enemybullets.begin()+idx);
            idx--;
        }
    }
    for(int idx=0;idx<enemies.size();idx++){//遍历敌人
        static Assets::Image enemyTexture;
        enemyTexture=Assets::Image(L"./enemy.bmp");
        enemy& nenemy=enemies[idx];
        nenemy.facing=atan2(pY-nenemy.y,pX-nenemy.x);
        Assets::Matrix enemyM(nenemy.facing);
        enemyTexture.matrix=enemyM;
        enemyTexture.transformation(false);
        painter.putImage(LOCATEMODE_CENTER,Point(nenemy.x,nenemy.y),enemyTexture,255);//同上玩家的绘制
        int deltaX=abs(nenemy.x-pX);
        int deltaY=abs(nenemy.y-pY);//计算位移
        int distance=sqrt(deltaX*deltaX+deltaY*deltaY);//距离
        vector<Point> hpbar={{nenemy.x-nenemy.HP*2,nenemy.y-30},{nenemy.x-nenemy.HP*2,nenemy.y-35},
                            {nenemy.x+nenemy.HP*2,nenemy.y-35},{nenemy.x+nenemy.HP*2,nenemy.y-30}};
        painter.solidPolygon(hpbar,Color((unsigned char)255,0,0,127));//同上方血量条
        //我爱死输入法了2.0
        //adjust movement speed based on difficulty(higher D -> faster)
        //根据难度增加速度
        double baseSpeed=3.0;//基准速度
        double moveSpeed=baseSpeed*(1.0+0.5*D);//矫正速度
        if(distance<20&&learnedSkills[L"美食家"]!=0){
            if(rand()%30<learnedSkills[L"美食家"]){
                playerHP+=learnedSkills[L"美食家"];
                enemies.erase(enemies.begin()+idx);
                idx--;
            }
        }
        else if(distance<190){
            nenemy.x-=(int)(moveSpeed*cos(nenemy.facing));
            nenemy.y-=(int)(moveSpeed*sin(nenemy.facing));
        }//离得远就飞过来
        else if(distance>=190&&distance<210){
            if(!(--nenemy.shootCooldown)){
                enemybullet newBullet(nenemy.x,nenemy.y,nenemy.facing);
                enemybullets.push_back(newBullet);
                int baseCd=rand()%20+50;
                nenemy.shootCooldown=std::max(1,(int)(baseCd/D));
            }
        }//距离适中就开始射击
        else{
            nenemy.x+=(int)(moveSpeed*cos(nenemy.facing));
            nenemy.y+=(int)(moveSpeed*sin(nenemy.facing));//否则后退
        }//敌人的AI部分
    }

    //prevent enemy stacking: simple separation impulse
    for(size_t i=0;i<enemies.size();i++){
        for(size_t j=i+1;j<enemies.size();j++){
            int dx=enemies[j].x-enemies[i].x;
            int dy=enemies[j].y-enemies[i].y;
            double dist=sqrt((double)dx*dx+(double)dy*dy);
            double minDist=40.0; //minimal separation
            if(dist>0&&dist<minDist){
                double overlap=(minDist-dist)/2.0;
                double nx=dx/dist;
                double ny=dy/dist;
                enemies[j].x+=(int)(nx*overlap);
                enemies[j].y+=(int)(ny*overlap);
                enemies[i].x-=(int)(nx*overlap);
                enemies[i].y-=(int)(ny*overlap);
            }
        }
    }//简单的碰撞检测防止敌机重叠

    {//我也不知道为什么我当时要套一层花括号
        //算了
        static Assets::Image enemyCollisionTexture;
        enemyCollisionTexture=Assets::Image(L"./enemy.bmp");
        int bw=(int)(bulletTexture.getWidth()/2);
        int bh=(int)(bulletTexture.getHeight()/2);
        int ew=(int)(enemyCollisionTexture.getWidth()/2);
        int eh=(int)(enemyCollisionTexture.getHeight()/2);
        for(int bi=0;bi<bullets.size();bi++){
            bool bulletRemoved=false;
            for(int ei=0;ei<enemies.size();ei++){
                auto &b=bullets[bi];
                auto &e=enemies[ei];
                if(b.x-bw<e.x+ew&&b.x+bw>e.x-ew&&b.y-bh<e.y+eh&&b.y+bh>e.y-eh){
                    GameLogger.traceLog(Core::logger::LOG_INFO,"Bullet hit enemy");
                    bullets.erase(bullets.begin()+bi);
                    e.HP-=5;
                    e.HP-=level;
                    if(learnedSkills[L"闪电链"]>0&&!enemies.empty()){
                        float decay=0.7f;
                        float extraDecay=1.0f;
                        enemy *chainedEnemy=&enemies[rand()%enemies.size()];
                        chainedEnemy->HP-=(5+level)*decay;
                        painter.line(Point(e.x+rand()%20,e.y+rand()%20),Point(chainedEnemy->x+rand()%20,chainedEnemy->y+rand()%20),Color((unsigned char)0,255,255,255));
                        int extraChain=learnedSkills[L"闪电链"]-1;
                        while(extraChain-->0){
                            decay*=0.7f;
                            enemy *chainedExtraEnemy=&enemies[rand()%enemies.size()];
                            if(chainedExtraEnemy==chainedEnemy)extraDecay=0.1f;
                            chainedExtraEnemy->HP-=(5+level)*decay*extraDecay;
                            extraDecay=1.0f;
                            painter.line(Point(chainedEnemy->x+rand()%20,chainedEnemy->y+rand()%20),Point(chainedExtraEnemy->x+rand()%20,chainedExtraEnemy->y+rand()%20),Color((unsigned char)0,255,255,255));
                            chainedEnemy=chainedExtraEnemy;
                        }
                    }
                    if(e.HP<=0){
                        EXPorb newExpOrb={e.x,e.y,rand()%4+3};
                        exporbs.push_back(newExpOrb);
                        enemies.erase(enemies.begin()+ei);
                    }
                    bulletRemoved=true;
                    break;
                }
            }
            if(bulletRemoved){
                bi--;
            }
        }//遍历子弹和敌人，处理子弹与敌人的碰撞
    }

    {//同上
        int pw=(int)(img.getWidth()/2);
        int ph=(int)(img.getHeight()/2);
        int ebw=(int)(enemyBulletTexture.getWidth()/2);
        int ebh=(int)(enemyBulletTexture.getHeight()/2);
        for(int ei=0;ei<enemybullets.size();ei++){
            auto &eb=enemybullets[ei];
            if(pX-pw<eb.x+ebw&&pX+pw>eb.x-ebw&&pY-ph<eb.y+ebh&&pY+ph>eb.y-ebh){
                GameLogger.traceLog(Core::logger::LOG_INFO,"Player hit by enemy bullet");
                enemybullets.erase(enemybullets.begin()+ei);
                ei--;
                playerHP-=4;
                if(rand()%20<learnedSkills[L"学习"]){
                    nowexp+=learnedSkills[L"学习"];
                }
                if(learnedSkills[L"应激"]){
                    reactDuration=60;
                }
                painter.drawBackground(Color((unsigned char)255,0,0,127));
            }
        }//也是子弹与飞机，只不过取not
    }

    {
        int pr=(int)(img.getWidth()/2);
        for(int i=0;i<exporbs.size();i++){
            auto &ex=exporbs[i];//经验球
            painter.solidCircle(Point(ex.x,ex.y),ex.exp,Color((unsigned char)255,255,0,255));//根据经验球的经验含量绘制不同半径的圆
            int dx=pX-ex.x;
            int dy=pY-ex.y;
            int rsum=pr+ex.exp;
            if(dx*dx+dy*dy<=rsum*rsum){
                nowexp+=ex.exp;
                GameLogger.formatLog(Core::logger::LOG_INFO,"Picked up exp: %d",ex.exp);//日志
                exporbs.erase(exporbs.begin()+i);
                i--;
                if(learnedSkills[L"拾荒"]){
                    playerHP+=1;
                    if(learnedSkills[L"拾荒"]>=3){
                        playerHP+=1;
                    }
                }
                while(nowexp>20){
                    nowexp-=20;
                    level++;//升级部分
                    GameLogger.formatLog(Core::logger::LOG_INFO,"Level up! Now level: %d",level);
                    playerHP=20+5*learnedSkills[L"加固护甲"];
                    //prepare skill choices: pick up to 3 random skills that are not yet at max level
                    std::vector<int> available;
                    for(int si=0; si<skillPool.size(); ++si){
                        int curLv=learnedSkills.count(skillPool[si].name)?learnedSkills[skillPool[si].name]:0;
                        if(curLv<skillPool[si].maxLevel) available.push_back(si);
                    }
                    if(!available.empty()){
                        std::random_device rd;
                        std::mt19937 g(rd());
                        std::shuffle(available.begin(),available.end(),g);
                        skillChoices.clear();
                        for(int k=0;k<3&&k<available.size(); ++k) skillChoices.push_back(available[k]);
                        selectingSkill=true;
                    }//从特殊技能中选择一个
                }
            }
        }
    }

    //如果正在选择技能，渲染选择弹窗并暂停主窗口交互
    if(selectingSkill&&skillChoices.size()>0){
        //半透明遮罩
        painter.drawBackground(Color((unsigned char)0,0,0,127));
        Assets::Font nameFont(L"Arial",20,8,FONTWEIGHT_BOLD,false,false,false);
        nameFont.loadFont();
        Assets::Font descFont(L"Arial",14,6,FONTWEIGHT_NORMAL,false,false,false);
        descFont.loadFont();
        Assets::Font romanFont(L"Arial",18,7,FONTWEIGHT_BOLD,false,false,false);
        romanFont.loadFont();
        int baseX=150;
        int baseY=200;
        int gap=180;
        for(int i=0;i<skillChoices.size();i++){
            int sx=baseX+i*gap;
            int sy=baseY;
            Skill &sk=skillPool[skillChoices[i]];
            //图标（假设为50x50）
            //说假设是因为我不知道以后50x50够不够用=(
            painter.putImage(LOCATEMODE_CENTER,Point(sx,sy),sk.icon,255);
            //名称
            std::wstring wname=sk.name;
            painter.putText(LOCATEMODE_CENTER,Point(sx,sy+40),nameFont,wname,Color((unsigned char)255,255,255,255));
            //描述-分两行显示（按空格尽量断开）
            std::wstring desc=sk.description;
            size_t mid=desc.size()/2;
            size_t split=desc.find_last_of(L' ',mid);
            if(split==std::wstring::npos) split=mid;
            std::wstring d1=desc.substr(0,split);
            std::wstring d2=desc.substr(split);
            if(!d2.empty()&&d2[0]==L' ') d2=d2.substr(1);
            painter.putText(LOCATEMODE_CENTER,Point(sx,sy+60),descFont,d1,Color((unsigned char)200,200,200,255));
            painter.putText(LOCATEMODE_CENTER,Point(sx,sy+80),descFont,d2,Color((unsigned char)200,200,200,255));
            //等级（罗马数字）
            //偷懒只写了1~5的罗马数字
            int curLv=learnedSkills.count(sk.name)?learnedSkills[sk.name]:0;
            std::wstring romanCur=(curLv>0)?toRoman(curLv):std::wstring(L"Not learned");
            std::wstring romanMax=toRoman(sk.maxLevel);
            wchar_t lvbuf[64];
            swprintf(lvbuf,64,L"%ls/%ls",romanCur.c_str(),romanMax.c_str());
            painter.putText(LOCATEMODE_CENTER,Point(sx,sy+105),romanFont,lvbuf,Color((unsigned char)255,255,0,255));
            wchar_t dibuf[64];
            float diff=sk.difficultyInfluence;
            swprintf(dibuf,64,L"仇恨影响：%.1f",diff);
            if(diff>0.0f)
                painter.putText(LOCATEMODE_CENTER,Point(sx,sy+125),romanFont,dibuf,Color((unsigned char)255,0,0,255));
            else
                painter.putText(LOCATEMODE_CENTER,Point(sx,sy+125),romanFont,dibuf,Color((unsigned char)0,255,0,255));
        }
        //不继续渲染后续游戏内容，暂停主窗口
        return 0;
    }

    //calculate part
    //personally I usually put the loop code in the paint function
    //'cause they're the same,but for your program's future don't
    //do this like me 'cause it would be horrible to debug.
    //For this is only a demo,I won't change my habit now.
    //计算部分
    //我个人习惯把计算和绘制放在一起
    //是的我知道这习惯很烂
    //别问，问就是easyx时期留下的坏习惯
    //都怪easyx:P
    pX+=nowSpeed*cos(arc);
    pY+=nowSpeed*sin(arc);//处理玩家移动
    if(nowSpeed>0)
    nowSpeed-=rand()%10==0?1:0;
    if(playerHP<=0){
        if(learnedSkills[L"应急能源"]!=0){
            learnedSkills[L"应急能源"]--;
            playerHP=1;
            painter.drawBackground(Color((unsigned char)255,255,0,127));
        }
        else{
            painter.drawBackground(Color((unsigned char)127,127,127,127));
            Assets::Font font(L"Arial",100,40,FONTWEIGHT_BLACK,false,false,false);
            font.loadFont();
            painter.putText(LOCATEMODE_CENTER,Point(350,200),font,L"You died!",Color((unsigned char)255,255,255,255));
            painter.putText(LOCATEMODE_CENTER,Point(350,400),font,L"Quitting...",Color((unsigned char)255,255,255,255));
            waitingForQuit=true;
        }
    }//死亡部分
    //处理僚机
    static Assets::Image wingmanTexture;
    static bool wingmanTexLoaded=false;
    if(!wingmanTexLoaded){
        wingmanTexture=Assets::Image(L"./extrap.bmp");
        wingmanTexLoaded=true;
    }

    //更新僚机数量（技能等级可能在游戏过程中变化）
    updateWingmans();

    for(auto& w:wingmans){
        if(enemies.empty()){
            //无敌人时跟随玩家
            w.facing=atan2(pY-w.y,pX-w.x);
            int dx=pX-w.x,dy=pY-w.y;
            int dist=sqrt(dx*dx+dy*dy);
            if(dist>100){
                w.x+=2*cos(w.facing);
                w.y+=2*sin(w.facing);
            } else if(dist<50){
                w.x-=cos(w.facing);
                w.y-=sin(w.facing);
            }
        }
        else{
            //寻找最近的敌人
            int nearestIdx=0;
            int minDist=999999;
            for(size_t i=0; i<enemies.size(); ++i){
                int dx=enemies[i].x-w.x,dy=enemies[i].y-w.y;
                int dist=dx*dx+dy*dy;
                if(dist<minDist){
                    minDist=dist;
                    nearestIdx=i;
                }
            }
            enemy& target=enemies[nearestIdx];
            w.facing=atan2(target.y-w.y,target.x-w.x);
            int dx=target.x-w.x,dy=target.y-w.y;
            int dist=sqrt(dx*dx+dy*dy);
            double baseSpeed=2.5;
            if(dist<150){
                //远离敌人
                w.x-=(int)(baseSpeed*cos(w.facing));
                w.y-=(int)(baseSpeed*sin(w.facing));
            }
            else if(dist>=150&&dist<200){
                //射击
                if(--w.shootCooldown<=0){
                    bullet newBullet(w.x,w.y,w.facing);
                    bullets.push_back(newBullet);
                    //攻速与玩家等级挂钩：基础冷却40，随等级减少
                    int baseCd=40;
                    w.shootCooldown=std::max(5,baseCd-level*3);
                }
            }
            else{
                //接近敌人
                w.x+=(int)(baseSpeed*cos(w.facing));
                w.y+=(int)(baseSpeed*sin(w.facing));
            }
        }

        //边界限制
        w.x=std::clamp(w.x,0,700);
        w.y=std::clamp(w.y,0,700);
        //std算法库太好用了你们知道吗

        //绘制僚机
        wingmanTexture=Assets::Image(L"./extrap.bmp");
        Assets::Matrix wm(w.facing);
        wingmanTexture.matrix=wm;
        wingmanTexture.transformation(false);
        painter.putImage(LOCATEMODE_CENTER,Point(w.x,w.y),wingmanTexture,255);
    }
    for(size_t i=0;i<wingmans.size();i++){
        for(size_t j=i+1;j<wingmans.size();j++){
            int dx=wingmans[j].x-wingmans[i].x;
            int dy=wingmans[j].y-wingmans[i].y;
            double dist=sqrt((double)dx*dx+(double)dy*dy);
            double minDist=40.0; //minimal separation
            if(dist>0&&dist<minDist){
                double overlap=(minDist-dist)/2.0;
                double nx=dx/dist;
                double ny=dy/dist;
                wingmans[j].x+=(int)(nx*overlap);
                wingmans[j].y+=(int)(ny*overlap);
                wingmans[i].x-=(int)(nx*overlap);
                wingmans[i].y-=(int)(ny*overlap);
            }
        }
    }
    //draw difficulty in bottom-right
    {
        Assets::Font df(L"Arial",18,8,FONTWEIGHT_NORMAL,false,false,false);
        df.loadFont();
        wchar_t diffbuf[64];
        swprintf(diffbuf,64,L"Difficulty: %.2f",D);
        painter.putText(LOCATEMODE_RIGHTBOTTOMCORNER,Point(695,700),df,diffbuf,Color((unsigned char)255,0,0,127));
    }
    return 0;
}

long long mainWindowMouseMove(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,int x,int y){
    arc=atan2(y-pY,x-pX);//calculate the angle that the spaceship is facing
    mX=x;
    mY=y;
    return 0;
}
long long mainWindowLHeld(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,int x,int y,unsigned long long time){
    if(learnedSkills[L"多向炮台"]){
        bullets.push_back(bullet(pX,pY,0));
        bullets.push_back(bullet(pX,pY,acos(-1.0)/2));
        bullets.push_back(bullet(pX,pY,acos(-1.0)));
        bullets.push_back(bullet(pX,pY,3*acos(-1.0)/2));
        if(learnedSkills[L"多向炮台"]>=2){
            bullets.push_back(bullet(pX,pY,acos(-1.0)/4));
            bullets.push_back(bullet(pX,pY,3*acos(-1.0)/4));
            bullets.push_back(bullet(pX,pY,5*acos(-1.0)/4));
            bullets.push_back(bullet(pX,pY,7*acos(-1.0)/4));
        }
    }
    return 0;
}
long long mainWindowRHeld(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,int x,int y,unsigned long long time){
    if(learnedSkills[L"增援"]>=5){
        for(auto& wingman:wingmans){
            wingman.x=pX+rand()%50;
            wingman.y=pY+rand()%50;
        }
    }
    return 0;
}
long long mainWindowLClick(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,int x,int y){
    //如果正在选择技能，优先处理技能选择点击
    if(selectingSkill&&skillChoices.size()>0){
        int baseX=150;
        int baseY=200;
        int gap=180;
        for(int i=0;i<skillChoices.size();i++){
            int sx=baseX+i*gap;
            int sy=baseY;
            //点击图标区域（50x50）则选中
            if(x>=sx-25&&x<=sx+25&&y>=sy-25&&y<=sy+25){
                int idx=skillChoices[i];
                std::wstring skname=skillPool[idx].name;
                int curLv=learnedSkills.count(skname)?learnedSkills[skname]:0;
                if(curLv<skillPool[idx].maxLevel){
                    learnedSkills[skname]=curLv+1;
                    std::string skn=toUTF8(skname);
                    GameLogger.formatLog(Core::logger::LOG_INFO,"Learned skill: %s,now level %d",skn.c_str(),learnedSkills[skname]);
                }
                selectingSkill=false;
                skillChoices.clear();
                playerHP=20+5*learnedSkills[L"加固护甲"];
                updateWingmans();  //根据新的增援等级调整僚机数量
                totalRage+=skillPool[idx].difficultyInfluence;
                return 0;
            }
        }
        //点击其他位置不发射子弹也不关闭选择
        return 0;
    }
    if(shootCooldown) return 0;//还在冷却就退出
    bullet newBullet(pX,pY,arc);
    bullets.push_back(newBullet);//create new bullet
    int multishot=learnedSkills[L"多重射击"];
    for(int i=1;i<=multishot;i++){
        bullets.push_back(bullet(pX,pY,arc+i*(acos(-1.0)/12)));
        bullets.push_back(bullet(pX,pY,arc-i*(acos(-1.0)/12)));
    }
    //发射子弹
    shootCooldown=std::max(50-(5*level),10)-learnedSkills[L"攻速突破"];
    return 0;
}

long long mainWindowRClick(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,int x,int y){
    if(learnedSkills[L"隧穿"]){
        pX+=learnedSkills[L"隧穿"]*60*cos(arc);
        pY+=learnedSkills[L"隧穿"]*60*sin(arc);
        return 0;
    }
    nowSpeed=7+learnedSkills[L"推进引擎"]+static_cast<bool>(reactDuration)*2*learnedSkills[L"应激"]-learnedSkills[L"沉重核心"]*3;//移动初速度
    return 0;
}

int main(){
    srand(time(nullptr));//随机种子
    for(int i=0;i<100;i++){
        stars.push_back(std::make_pair(Point(rand()%700,rand()%700),rand()%255));//星星背景
    }
    auto mainWindow=createInitWindow(200,200,707,730,L"Window");//创建窗口
    //函数返回值为 std::pair<std::shared_ptr<Handle>,HWND>，其中Handle是我图形库的窗口类
    mainWindow.first->thisPaint=mainWindowPaint;
    mainWindow.first->thisMouseMove=mainWindowMouseMove;
    mainWindow.first->thisInstantRightClick=mainWindowRClick;
    mainWindow.first->thisLeftHeld=mainWindowLHeld;
    mainWindow.first->thisRightHeld=mainWindowRHeld;
    mainWindow.first->thisInstantLeftClick=mainWindowLClick;//inject the functions into the windows
    //注入窗口回调
    MSG msg={};
    while(msg.message!=WM_QUIT){//主循环
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(!(--enemySpawnCooldown)){//依旧OIER特有的
            enemy newEnemy;
            newEnemy.x=rand()%700;
            newEnemy.y=(rand()%2)*700;
            newEnemy.facing=atan2(pY-newEnemy.y,pX-newEnemy.x);
            //scale shoot cooldown and HP by current difficulty(inverse for cooldown)
            double safeD=std::max(0.1,currentDifficulty);
            newEnemy.shootCooldown=std::max(1,(int)(60.0/safeD));
            newEnemy.HP=20+(int)(10.0*currentDifficulty);
            enemies.push_back(newEnemy);
            enemySpawnCooldown=std::max(5,(int)(400.0/std::max(0.1,1+currentDifficulty)));
        }
        Sleep(16);//~60fps
        globalHandleManager.updateAll();//literally
        if(waitingForQuit){
            Sleep(4000);
            exit(0);
        }
    }
}
