#include "common.h"
#include "utils.h"
using namespace std;
World world;

vector<ResourceEnum> RESOURCES = 
{ 
    ResourceEnum::Wood,
    ResourceEnum::Stone,
    ResourceEnum::Iron,
    ResourceEnum::Gem,
    ResourceEnum::Wool,
    ResourceEnum::Hide,
    ResourceEnum::Wheat,
    ResourceEnum::Pineapple,
    ResourceEnum::Gold 
};//vsetky resourcy

unordered_map<int, ShipsEnum> classTradeMap = 
{
    {10,ShipsEnum::Cln},
    {30,ShipsEnum::Plt},
    {100,ShipsEnum::SmallMerchantShip},
    {200,ShipsEnum::LargeMerchantShip}
};//cena trde lodi
unordered_map<int, ShipsEnum> classAttackMap = 
{
    {15,ShipsEnum::SomalianPirateShip},
    {50,ShipsEnum::BlackPearl},
    {30,ShipsEnum::SniperAttackShip},
};//cena utocnej lodi
unordered_map<int, ShipsEnum> classLootMap = 
{
    {50,ShipsEnum::LooterScooter}
};//cena loot lodi

unordered_map<XY,vector<XY>> medzera = 
{
    {XY{0,0},{}},
    {XY{1,0},{}},
    {XY{0,1},{}},
    {XY{-1,0},{}},
    {XY{0,-1},{}},
    {XY{2,0},{{1,0}}},
    {XY{-2,0},{{-1,0}}},
    {XY{0,2},{{0,1}}},
    {XY{0,-2},{{0,-1}}},
    {XY{3,0},{{2,0},{1,0}}},
    {XY{-3,0},{{-2,0},{-1,0}}},
    {XY{0,3},{{0,2},{0,1}}},
    {XY{0,-3},{{0,-2},{0,-1}}},
    //jenden smer
    {XY{1,1},{{1,0},{0,1}}},
    {XY{-1,1},{{-1,0},{0,1}}},
    {XY{1,-1},{{1,0},{0,-1}}},
    {XY{-1,-1},{{-1,0},{0,-1}}},
    //dva smery    
    {XY{2,1},{{1,0},{2,0},{1,0},{1,1},{0,1},{1,1}}},
    {XY{2,-1},{{1,0},{2,0},{1,0},{1,-1},{0,-1},{1,-1}}},
    {XY{-2,1},{{-1,0},{-2,0},{-1,0},{-1,1},{0,1},{-1,1}}},
    {XY{-2,-1},{{-1,0},{-2,0},{-1,0},{-1,-1},{0,-1},{-1,-1}}},
    {XY{1,2},{{0,1},{0,2},{0,1},{1,1},{1,0},{1,1}}},
    {XY{-1,2},{{0,1},{0,2},{0,1},{-1,1},{-1,0},{-1,1}}},
    {XY{1,-2},{{0,-1},{0,-2},{0,-1},{1,-1},{1,0},{1,-1}}},
    {XY{-1,-2},{{0,-1},{0,-2},{0,-1},{-1,-1},{-1,0},{-1,-1}}}
    //tri smery
};//policka medzi dvoma bodmi

struct Trade{
    double odhad;
    Harbor FromH;
    Harbor ToH;
    int tovar;

    
}t;

struct newTrade{
    double odhad;
    XY FromH;
    XY ToH;
    int tovar;
    int pocet;
}nt;

struct infoH{
    Harbor harbor;
    vector<int> storage;
    vector<int> production;
    vector<int> alocated;

    void init(vector<int> &storage, vector<int> &production){
        storage.resize(8,0);
        production.resize(8,0);
        alocated.resize(8,0);
    }
}iH;


vector<vector<int>> mapka,mapkaD;
vector<pair<XY,bool>> coords_of_all_harbors; //bool = ci uz som ho nasiel
vector<Harbor> vector_of_found_harbors; //nasiel som
unordered_map<XY,Harbor> mapka_harbors; //mapka pristavov
vector<vector<pair<int,Harbor>>> production_of_harbors(8),consumption_of_harbours(8); //produkcia pristavov, konzumacia pristavov
vector<pair<Ship,pair<int,Trade>>> ship_orders; //,0=default, 1=predat, 2=kupit, 3=utocit,4=explore,5=calculate,6=stacionarne,7=stoji
unordered_map<int, pair<Ship,pair<int,newTrade>>> shipOrders; //,0=default, 1=predat, 2=kupit, 3=utocit,4=explore,5=calculate,6=stacionarne,7=stoji
vector<Trade> trades; //vsetky trades
vector<pair<Harbor,int>> occupied_harbors; //obsadene pristavy
vector<pair<int,XY>> destinations; //destinacie lodiek
unordered_map<XY,infoH> storageHarborsMap; //storage a prod pristavov


int tah=0; //pocitam si tahy
bool trebaExplorovat = true; //ci treba explorovat
int indexExplorer=-1;

int distance(XY& a, XY& b){
    return abs(a.x - b.x) + abs(a.y - b.y); 
} //vzdialenost dvoch bodov
int manhattanDistance(int x1, int y1, int x2, int y2) {
  return abs(x1 - x2) + abs(y1 - y2);
} //manhattan vzdialenost
vector<vector<int>> manhattanBody(int n,int m, int x, int y, int vzdialenost) {
  vector<vector<int>> body;

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      if (manhattanDistance(i, j, x, y) <= vzdialenost) {
        body.push_back({i, j});
      }
    }
  }

  return body;
}//vrati vsetky body v manhattan vzdialenosti a menej
vector<XY> ziskanieSmery(Ship ship){
    vector<XY> smery;
    int vzdialenost=ship.stats.max_move_range;
    int n=1000,m=1000,x=12,y=12;
    vector<vector<int>> body = manhattanBody(n,m, x, y, vzdialenost);
    for (vector<int> bod : body) {
        if(XY{bod[1]-12,bod[0]-12}!=XY{0,0})
        smery.push_back(XY(bod[0]-12,bod[1]-12));
    }
    // std::vector<XY> ADJ{{1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {2,0}, {-2,0}, {0,2}, {0,-2}};

    return smery;
    
}//vrati vsetky smery v manhattan vzdialenosti a menej
ShipsEnum getShipName(Ship ship){
    if(ship.stats.ship_class==ShipClass::SHIP_TRADE)
        return classTradeMap[ship.stats.price];
    if(ship.stats.ship_class==ShipClass::SHIP_ATTACK)
        return classAttackMap[ship.stats.price];
    if(ship.stats.ship_class==ShipClass::SHIP_LOOT)
        return classLootMap[ship.stats.price];
    return ShipsEnum::Cln;
}//vrati meno lode
bool medzi(XY a,XY b){
    XY nieco=XY{b.x-a.x,b.y-a.y};
    if(abs(nieco.x)==1&&abs(nieco.y)==1){
        for(int i=0;i<2;i++){
            bool tmp=true;
            XY najdene=XY{a.x+medzera[nieco][i*2].x,a.y+medzera[nieco][i*2].y};
            if(mapka[najdene.y][najdene.x]==2)tmp= false;
            if(tmp)return true;
        }
        return false;
    }

    if(abs(nieco.x)==1&&abs(nieco.y)==2){
        for(int i=0;i<3;i++){
            bool tmp=true;
            for(int j=0;j<2;j++){
                XY najdene=XY{a.x+medzera[nieco][i*2+j].x,a.y+medzera[nieco][i*2+j].y};
                if(mapka[najdene.y][najdene.x]==2)tmp= false;
                
            }
            if(tmp)return true;
        }
        return false;
    }
    if(abs(nieco.x)==2&&abs(nieco.y)==1){
        for(int i=0;i<3;i++){
            bool tmp=true;
            for(int j=0;j<2;j++){
                XY najdene=XY{a.x+medzera[nieco][i*2+j].x,a.y+medzera[nieco][i*2+j].y};
                if(mapka[najdene.y][najdene.x]==2)tmp=false;
            }
            if(tmp)return true;
        }
        return false;
    }
    bool tmp=true;
    for(int j=0;j<medzera[nieco].size();j++){
        XY najdene=XY{a.x+medzera[nieco][j].x,a.y+medzera[nieco][j].y};
        if(mapka[najdene.y][najdene.x]==2)tmp= false;            
    }
    if(tmp)return true;
    return false;

}//ci je medzi bodmi cesta
bool condition(XY a, XY b) {
    if(b.x < 0 || b.x >= world.mapa.width || b.y < 0 || b.y >= world.mapa.height) return false;
    if(mapka[b.y][b.x]==2||mapka[b.y][b.x]==5) return false;
    if(medzi(a,b)==false) return false;
    return true;
}//ci sa mozem pohnut na dany bod
int manyToBuy(pair<Ship,pair<int,Trade>> s,ResourceEnum res,int pocet){
    int cost=s.second.second.FromH.resource_cost(res);
    int maximalnyPocet=s.first.resources.resources[8]/cost;
    if(pocet>maximalnyPocet) return pocet-1;
    return manyToBuy(s,res,pocet+1);
}//vypocitq kolko resourcov kupit
int pocetLodiciek(ShipsEnum lodka){
    int pocet=0;
    for(Ship ship:world.my_ships()){
        if(getShipName(ship)==lodka) pocet++;
    }
    return pocet;
}//pocet lodiek daneho typu

int resourceCost(int what, XY where,bool alocovat = true) {
        int amount = storageHarborsMap[where].storage[what];
        // if(alocovat) amount -=storageHarborsMap[where].alocated[what];
        vector<int> resourceCost{1, 2, 5, 10, 3, 5, 2, 3};
        int ret = min(100 / (amount + 3) + 1, 4) * resourceCost[what];
        return ret;
    }
int newManyToBuy( pair<Ship,pair<int,newTrade>> s,int pocet){
    int cost=mapka_harbors[s.first.coords].resource_cost(RESOURCES[s.second.second.tovar]);
    int maximalnyPocet=s.first.resources.resources[8]/cost;
    if(pocet>maximalnyPocet) return pocet-1;
    return newManyToBuy(s,pocet+1);
}//vypocitq kolko resourcov kupit

newTrade newCalculateTrade(XY from,XY to,Ship ship,int res){
    cerr<<"newCalculateTrade"<<endl;
    int cargoGold;
    double priceBuy,priceSell;
    double vzdialenost;
    double amount;

    double realProfit;

    cargoGold=ship.resources.resources[8];
    priceBuy=resourceCost(res,from);
    priceSell=resourceCost(res,to);
    //TODO: distacne s bfs
    vzdialenost=distance(world.my_base,from)+distance(from,to)+distance(to,world.my_base);
    amount=storageHarborsMap[from].storage[res];
    // -storageHarborsMap[from].alocated[res];
    realProfit=min(amount,cargoGold/priceBuy)*priceSell;

    newTrade tmp;
    tmp.FromH=from;
    tmp.ToH=to;
    tmp.tovar=res;
    tmp.pocet=min(amount,cargoGold/priceBuy);
    tmp.odhad=vzdialenost/realProfit;
    cerr<<"priceBuy"<<priceBuy<<" priceSell"<<priceSell<<" vzdialenost"<<vzdialenost<<" amount"<<amount<<" realProfit"<<realProfit<<endl;
    cerr<<"newtmp.odhad"<<tmp.odhad<<endl;
    return tmp;
    

}
newTrade findNewTrade(Ship ship){
    cerr<<"findNewTrade"<<endl;
    newTrade best;
    best.odhad=1000000;
    for(int i=0;i<vector_of_found_harbors.size();i++){
        for(int j=i+1;j<vector_of_found_harbors.size();j++){
            for(int k=0;k<8;k++){
                XY prve=vector_of_found_harbors[i].coords;
                XY druhe=vector_of_found_harbors[j].coords;
                int sotrageP=storageHarborsMap[prve].storage[k];
                // -storageHarborsMap[prve].alocated[k];
                int sotrageD=storageHarborsMap[druhe].storage[k];
                // -storageHarborsMap[druhe].alocated[k];
                cerr<<"findNewTrade"<<sotrageP<<" "<<sotrageD<<endl;
                if(sotrageD<sotrageP&&storageHarborsMap[druhe].production[k]<0){
                    newTrade tmp=newCalculateTrade(prve,druhe,ship,k);
                    if(tmp.odhad<best.odhad){
                        best=tmp;
                    }
                }
                else if(sotrageD>sotrageP&&storageHarborsMap[prve].production[k]>0){
                    newTrade tmp=newCalculateTrade(druhe,prve,ship,k);
                    if(tmp.odhad<best.odhad){
                        best=tmp;
                    }
                }
            }

        }
    }
    if(best.odhad!=1000000){    
        // storageHarborsMap[best.FromH].alocated[best.tovar]+=best.pocet;
        // storageHarborsMap[best.ToH].alocated[best.tovar]-=best.pocet;     
    }
    cerr<<"findBest.odhad"<<best.odhad<<endl;
    return best;
    
}

XY closest(XY destination,Ship ship){
    if(ship.coords == destination) return destination;
    // if(mapka[destination.y][destination.x]==0) return destination;
    if(mapka[destination.y][destination.x]!=2&&mapka[destination.y][destination.x]!=5) return destination;

    int vzdialenost = 1;
    while(true){
        vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, destination.x, destination.y, vzdialenost);
        for(vector<int> bod : body){
            if(mapka[bod[1]][bod[0]]!=2&&mapka[bod[1]][bod[0]]!=5) return XY(bod[0],bod[1]);
        }
        vzdialenost++;
    }
}//najblizsi bod kam pohnut
void makeTradeProd(int tovar,Harbor harbor){
    cerr<<"makeTradeProd"<<"tovar: "<<tovar<<endl;
    for(auto consumption:consumption_of_harbours[tovar]){

        Trade trade;
        trade.FromH=harbor;
        trade.ToH=consumption.second;
        trade.tovar=tovar;
        int dis=distance(world.my_base,harbor.coords);
        dis+=distance(harbor.coords,consumption.second.coords);
        dis+=distance(consumption.second.coords,world.my_base);
        int profit=harbor.production.resources[tovar]-consumption.first;
        double tmp=dis/profit;
        trade.odhad=tmp;
        trades.push_back(trade);

    }
}//vytvori trade s pristavmi co konzumuju
void makeTradeCons(int tovar,Harbor harbor){
    for(auto production:production_of_harbors[tovar]){
        Trade trade;
        trade.FromH=production.second;
        trade.ToH=harbor;
        trade.tovar=tovar;
        int dis=distance(world.my_base,production.second.coords);
        dis+=distance(production.second.coords,harbor.coords);
        dis+=distance(harbor.coords,world.my_base);
        int profit=production.first-harbor.production.resources[tovar];
        double tmp=dis/profit;
        trade.odhad=tmp;
        trades.push_back(trade);

    }

}//vytvori trade s pristavmi co produkuju
void zijuciExplorer(){

    if(world.my_ships().size()==0) return;
    for(int i=0;i<shipOrders.size();i++){
        if(shipOrders[i].second.first == 0&&shipOrders[i].first.stats.max_cargo==10){
            shipOrders[i].second.first = 4;
            cerr<<"explorer: "<<shipOrders[i].first.index<<endl;
            indexExplorer=shipOrders[i].first.index;
            return;
        }
    }


    /*//old
    cerr<<"zijuciExplorer"<<endl;
    if(world.my_ships().size()==0) return;
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 0&&ship_orders[i].first.stats.max_cargo==10){
            ship_orders[i].second.first = 4;
            cerr<<"explorer: "<<ship_orders[i].first.index<<endl;
            return;
        }
    }
    //old*/
}//nastavi explorera
void updateGold(vector<Turn>& turns, Ship ship){
    if(ship.coords == world.my_base){

        if(ship.resources.resources[8] > ship.stats.max_cargo)
        turns.push_back(StoreTurn(ship.index, ship.resources.resources[8] - ship.stats.max_cargo));
        else
        turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo, world.gold)));
        return;
    }
    vector<XY> smery = ziskanieSmery(ship);
    turns.push_back(MoveTurn(ship.index, move_to(ship, closest(world.my_base,ship), condition, smery)));
    return;
}//zoberiem zlato z baseky
//pomocne funkcie

void Explore(vector<Turn>& turns, Ship ship){
    if(world.mapa.tiles[ship.coords.y][ship.coords.x].type == TileEnum::TILE_HARBOR){
            for(Harbor harbor : world.harbors){
                if(harbor.coords == ship.coords){
                    vector_of_found_harbors.push_back(harbor);
                    mapka_harbors[harbor.coords]=harbor;
                    if(vector_of_found_harbors.size() == world.harbors.size()) trebaExplorovat = false;
                    for(int i=0;i<8;i++){
                        if(harbor.production.resources[i]>0){
                            production_of_harbors[i].push_back(make_pair(harbor.production.resources[i],harbor));
                            makeTradeProd(i,harbor);

                        }

                        if(harbor.production.resources[i]<0){
                            consumption_of_harbours[i].push_back(make_pair(harbor.production.resources[i],harbor));
                            makeTradeCons(i,harbor);
                        }
                    }//zapisem si produkciu a konzumaciu pristavov
                    sort(trades.begin(),trades.end(),[](Trade a,Trade b){return a.odhad<b.odhad;});
                    cerr<<"trades:"<<trades.size()<<endl;
                    for(int i=0;i<coords_of_all_harbors.size();i++){
                        if(coords_of_all_harbors[i].first == harbor.coords){
                            coords_of_all_harbors[i].second = true;
                            cerr<<"nasiel som:"<<harbor.coords.x<<" "<<harbor.coords.y<<endl;
                            break;
                        }
                    }//oznacim si ze som ho nasel
                    
                }
                
            }
            
            
        
        }//ak sa nahcadzam na pristave tak si ho zapisem
            if(vector_of_found_harbors.size() == world.harbors.size())return;
            cerr<<"ship:"<<ship.coords.x<<" "<<ship.coords.y<<endl;
            unordered_map<XY, pair<int, XY>> dist;
            vector<Harbor> harbors = world.harbors;
            bfs(ship.coords, condition, dist);
            int min_dist = 1000000;
            XY min_harbor;
            for(int i=0;i<harbors.size();i++){
                if(dist[harbors[i].coords].first < min_dist&&coords_of_all_harbors[i].second==false){
                    min_dist = dist[harbors[i].coords].first;
                    min_harbor = harbors[i].coords;
                }
                
            }
            vector<XY> smery = ziskanieSmery(ship);

            turns.push_back(MoveTurn(ship.index, move_to(ship, closest(min_harbor,ship), condition, smery)));
            return;
        
}// najdem najblizsi pristav a pohnem sa na neho
void Attack(vector<Turn>& turns, Ship ship){
} //pohyb utocnej lode
void Calculate(vector<Turn>& turns, Ship ship){ 
    if(ship.resources.resources[8] > ship.stats.max_cargo) updateGold(turns, ship);
    else if(ship.resources.resources[8] < ship.stats.max_cargo&&world.gold!=0) updateGold(turns, ship);
    else{


        newTrade trade = findNewTrade(ship);
        if(trade.odhad!=1000000){
            shipOrders[ship.index].second.first = 2;
            shipOrders[ship.index].second.second=trade;
            vector<XY> smery = ziskanieSmery(ship);
            turns.push_back(MoveTurn(ship.index, move_to(ship, closest(trade.FromH,ship), condition,smery)));
            return;
        }
        else Explore(turns,ship);


    }
    return;
    

}//vypocitanie tradu
void Buy(vector<Turn>& turns, Ship ship){
    
    if(shipOrders[ship.index].first.coords==shipOrders[ship.index].second.second.FromH){
        int kolko=newManyToBuy(shipOrders[ship.index],1);
        turns.push_back(TradeTurn(ship.index, RESOURCES[shipOrders[ship.index].second.second.tovar], kolko));
        int pocet=shipOrders[ship.index].second.second.pocet;
        // storageHarborsMap[shipOrders[ship.index].second.second.FromH].alocated[shipOrders[ship.index].second.second.tovar]-=pocet;
        // storageHarborsMap[shipOrders[ship.index].second.second.ToH].alocated[shipOrders[ship.index].second.second.tovar]+=pocet;
        //TODO: alocated podla tho kolko kupil
        shipOrders[ship.index].second.first = 1;

    }
    else{
        vector<XY> smery = ziskanieSmery(ship);
        turns.push_back(MoveTurn(ship.index, move_to(ship, closest(shipOrders[ship.index].second.second.FromH,ship), condition,smery)));
    }
    
    //old
    /*int f=0;
    for(auto i:ship_orders){
    if(i.first.index == ship.index){
    if(ship.coords == i.second.second.FromH.coords){
        ResourceEnum res = RESOURCES[i.second.second.tovar];
        int kolko=manyToBuy(i,res,1);
        turns.push_back(TradeTurn(ship.index, res, kolko));
        ship_orders[f].second.first = 1;
        return;
    }
    else{
        cerr<<"suradky: "<<i.second.second.FromH.coords.x<<" "<<i.second.second.FromH.coords.y<<endl;
            vector<XY> smery = ziskanieSmery(ship);

        turns.push_back(MoveTurn(ship.index, move_to(ship, closest(i.second.second.FromH.coords,ship), condition,smery)));
        return;
        }
    }
    f++;
    }*/
    //old
}//kupovanie resourcov
void Sell(vector<Turn>& turns, Ship ship){

    if(shipOrders[ship.index].first.coords==shipOrders[ship.index].second.second.ToH){
        int kolko=ship.resources.resources[shipOrders[ship.index].second.second.tovar];
        turns.push_back(TradeTurn(ship.index, RESOURCES[shipOrders[ship.index].second.second.tovar], -kolko));
        // storageHarborsMap[shipOrders[ship.index].second.second.ToH].alocated[shipOrders[ship.index].second.second.tovar]+=kolko;
        shipOrders[ship.index].second.first = 5;
        return ;
    }
    else {
        vector<XY> smery = ziskanieSmery(ship);
        turns.push_back(MoveTurn(ship.index, move_to(ship, closest(shipOrders[ship.index].second.second.ToH,ship), condition,smery)));
    }


    //old
    /*int f=0;
    for (auto i:ship_orders){
    if(i.first.index ==ship.index){
        if(ship.coords == i.second.second.ToH.coords){
            ResourceEnum res = RESOURCES[i.second.second.tovar];
            int kolko=ship.resources[res];
            turns.push_back(TradeTurn(ship.index, res, -kolko));
            ship_orders[f].second.first = 5;
            return;
        }
        else{
                vector<XY> smery = ziskanieSmery(ship);

            turns.push_back(MoveTurn(ship.index, move_to(ship, closest(i.second.second.ToH.coords,ship), condition,smery)));
            return;
        }
    }
    f++;
    //old

}*/



}//predavanie resourcov
//hlavny funkcie

void updateShips(vector<Ship> ships){
cerr<<"updateShips"<<endl;
vector<pair<Ship,pair<int,Trade>>> new_ship_orders;
unordered_map<int, pair<Ship,pair<int,newTrade>>> new_shipOrders;
bool expoler =false;

for(Ship ship:ships){
    if(shipOrders.find(ship.index)==shipOrders.end()){
        if(ship.is_wreck) continue;
        pair<Ship,pair<int,newTrade>> tmp;
        tmp.first=ship;
        tmp.second.first=0;
        newTrade tmp2;
        tmp2.tovar=10;
        tmp.second.second=tmp2;
        shipOrders[ship.index]=tmp;
        if(shipOrders[ship.index].second.first == 4){
            expoler = true;
            indexExplorer=ship.index;
        } 

    }
    else{
        if(ship.is_wreck){
            if(shipOrders[ship.index].second.second.tovar!=10){
            // storageHarborsMap[shipOrders[ship.index].second.second.FromH].alocated[shipOrders[ship.index].second.second.tovar]+=shipOrders[ship.index].second.second.pocet;
            // storageHarborsMap[shipOrders[ship.index].second.second.ToH].alocated[shipOrders[ship.index].second.second.tovar]-=shipOrders[ship.index].second.second.pocet;
            }
            shipOrders.erase(ship.index);
            continue;
            
        }
        else {
            shipOrders[ship.index].first=ship;
            if(shipOrders[ship.index].second.first == 4){
            expoler = true;
            indexExplorer=ship.index;
        } 
            }
    }
}
    if(!expoler&&trebaExplorovat) zijuciExplorer();




} //Prida now lode do ship_orders a pridam explorera ak ho nemam
//old
//update ship_orders
void allHarbours(){
    for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(make_pair(harbor.coords,false));
            occupied_harbors.push_back(make_pair(harbor,0));
            cerr<<"Zharbor: "<<harbor.coords.y<<" "<<harbor.coords.x<<" - "<<endl;
        }
            
} //da vsetky pristavy do vectora coords_of_all_harbors a occupied_harbors
void createMap(){
    mapkaD.resize(world.mapa.height,vector<int> (world.mapa.width,0));

    for(int i=0;i<world.mapa.height;i++){
        for(int j=0;j<world.mapa.width;j++){

            switch (world.mapa.tiles[i][j].type)
            {
                case TileEnum::TILE_GROUND:
                    mapkaD[i][j]=2;
                    break;

                case TileEnum::TILE_BASE:
                    if(world.my_base!=XY(j,i)){  
                    vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, j, i, 4);
                    for (vector<int> bod : body) {
                        if(mapkaD[bod[1]][bod[0]]!=1&&mapkaD[bod[1]][bod[0]]!=3)
                        mapkaD[bod[1]][bod[0]]=2;
                        }
                        mapkaD[i][j]=3;
                }
                else mapkaD[i][j]=3;
                    break;

                case TileEnum::TILE_HARBOR:
                    mapkaD[i][j]=1;
                    vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, j, i, 8);
                    for (vector<int> bod : body) {
                        if(mapkaD[bod[1]][bod[0]]==0)
                        mapkaD[bod[1]][bod[0]]=4;
                        }
                        mapkaD[i][j]=1;
                    break;

            }            
        }
        
    }
    for(auto i:mapkaD){
        for(auto j:i){
            cerr<<j;
        }
        cerr<<endl;
    }//vypisem si mapku
    mapka=mapkaD;
}//vytvorim si mapkuD
void sortnutieOccupied(){
    cerr<<"sortnutieOccupied"<<endl;
        unordered_map<XY, pair<int, XY>> dist;
        cerr<<"nefunkcen bfs"<<endl;
        bfs(world.my_base, condition, dist);
        cerr<<"bfs"<<endl;
        for(int i=0;i<occupied_harbors.size();i++){
            occupied_harbors[i].second=dist[occupied_harbors[i].first.coords].first; 
        }
        cerr<<"sort"<<endl;
        sort(occupied_harbors.begin(),occupied_harbors.end(),[](pair<Harbor,int> a,pair<Harbor,int> b){return a.second<b.second;});        
}//zistim vzdialenosti pristavou od base a zoradim ich od najblizsieho
//iba v prvom kole
void updateMap(){
    cerr<<"updateMap"<<endl;
    mapka=mapkaD;
    for(Ship ship:world.ships){
        if(mapka[ship.coords.y][ship.coords.x]==0||mapka[ship.coords.y][ship.coords.x]==4)
        mapka[ship.coords.y][ship.coords.x]=5;
    }
}//aktualizujem mapku podla lodiek
//update mapy
void simulovanie(){
    cerr<<"simulovanie"<<endl;
    for(auto i:world.harbors){
        if(storageHarborsMap.find(i.coords)==storageHarborsMap.end()&&i.visible){
            cerr<<"init"<<endl;
            infoH info;
            info.init(info.storage,info.production);
            info.harbor=i;
            for(int j=0;j<8;j++){
                info.storage[j]=i.storage.resources[j];
                info.production[j]=i.production.resources[j];
            }
            storageHarborsMap[i.coords]=info;
        }
        else if(i.visible){
            for(int j=0;j<8;j++){
                storageHarborsMap[i.coords].storage[j]=i.storage.resources[j];
                storageHarborsMap[i.coords].production[j]=i.production.resources[j];//nemusim toto robit
            }

        }
        else if(storageHarborsMap.find(i.coords)!=storageHarborsMap.end()) {
            for(int j=0;j<8;j++){
                storageHarborsMap[i.coords].storage[j]
                +=storageHarborsMap[i.coords].production[j];
                if(storageHarborsMap[i.coords].storage[j]<0) storageHarborsMap[i.coords].storage[j]=0;
            }
        }
    }

}//odsimuluje produkciu harbov
//simulovanie
void umrtvitExplorera(vector<Turn>& turns){
    cerr<<shipOrders[indexExplorer].first.index;
    shipOrders[indexExplorer].second.first = 5;
    cerr<<"umrtvil som explorera"<<indexExplorer<<endl;



    /*//old
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 4){
            ship_orders[i].second.first = 7;
            cerr<<"umrtvil som explorera"<<ship_orders[i].first.index<<endl;
                vector<XY> smery = ziskanieSmery(ship_orders[i].first);

            turns.push_back(MoveTurn(ship_orders[i].first.index, move_to(ship_orders[i].first, closest(occupied_harbors[0].first.coords,ship_orders[i].first), condition,smery)));
            destinations.push_back(make_pair(ship_orders[i].first.index,occupied_harbors[0].first.coords));
            occupied_harbors.erase(occupied_harbors.begin());
            return;
        }
    }   
    //old*/
}//zrusim explorera
//explorer stuff
//TODO:
//kupovanie lodiciek
void updateOrders(){

    for(auto i:shipOrders){
        if(i.second.second.first == 0){
            if(shipOrders[i.first].first.stats.ship_class==ShipClass::SHIP_ATTACK) shipOrders[i.first].second.first = 3;
            if(shipOrders[i.first].first.stats.ship_class==ShipClass::SHIP_TRADE) shipOrders[i.first].second.first = 5;  
            if(getShipName(i.second.first)==ShipsEnum::Cln) shipOrders[i.first].second.first = 5;
        }
    }

    //old
   /* for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 0){
            if(ship_orders[i].first.stats.ship_class==ShipClass::SHIP_ATTACK) ship_orders[i].second.first = 3;
            if(ship_orders[i].first.stats.ship_class==ShipClass::SHIP_TRADE) ship_orders[i].second.first = 5;  
            if(getShipName(ship_orders[i].first)==ShipsEnum::Cln) ship_orders[i].second.first = 6;
        }
    }
    //old*/
}//da lodkam defaultny order
//update orders
//do turn funkcie 




void add_ship_turns(vector<Turn>& turns, vector<Ship> ships){
    for(Ship curr : ships){
        switch (shipOrders[curr.index].second.first)
        {
            case 4:
                cerr<<"explorujem"<<curr.index<<endl;
                Explore(turns,curr);
                break;
            case 3:
                cerr<<"utocim"<<curr.index<<endl;
                Attack(turns,curr);
                break;
            case 2:
                cerr<<"kupim"<<curr.index<<endl;
                Buy(turns,curr);
                break;
            case 1:
                cerr<<"predam"<<curr.index<<endl;
                Sell(turns,curr);
                break;
            case 5:
                cerr<<"pocitam"<<curr.index<<endl;
                Calculate(turns,curr);
                break;
            case 6:
                cerr<<"stacionarne"<<curr.index<<endl;
                break;
            case 7:
                cerr<<"stoji"<<curr.index<<endl;
                break;
            default:
                cerr<<"nic"<<curr.index<<endl;
                break;
            
        }
        
    }

}//pohyb lodi






vector<Turn> do_turn() {
    vector<Turn> turns;
    updateShips(world.my_ships());
    if(tah == 0) {
        allHarbours();
        createMap();
        sortnutieOccupied();
        }
    tah++;
    updateMap();
    simulovanie();
    //predpocitanie

    if(!trebaExplorovat) umrtvitExplorera(turns);
    //explorovanie

    if (pocetLodiciek(ShipsEnum::Cln) < 3) turns.push_back(BuyTurn(ShipsEnum::Cln));
    if (pocetLodiciek(ShipsEnum::Plt) < 0) turns.push_back(BuyTurn(ShipsEnum::Plt));
    // if(world.gold>=60&&pocetLodiciek(ShipsEnum::Cln)<4)  turns.push_back(BuyTurn(ShipsEnum::Cln));
    if(world.gold>=125)  turns.push_back(BuyTurn(ShipsEnum::SmallMerchantShip));

    //zaciatocne kupovanie lodiek
    updateOrders();
    //aktualizacia lodiek    

    add_ship_turns(turns, world.my_ships());
    //pohyb lodi

    
    //vypisy
    cerr << turns << endl;
    return turns;
}

int main() {
    char dot;
    while (1) {
        cin >> world >> dot;
        for (Turn turn : do_turn())
            cout << turn << "\n";
        cout << dot << endl;
    }
}



/*
vypisProduction();
vypisComsumption();
get_ship_resources(ship);
// Stacionarne(turns,curr);

*/

/*
void vypisProduction(){
    for(int i=0;i<8;i++){
        cerr<<"produkcne: "<<endl;
        for(int j=0;j<production_of_harbors[i].size();j++){
            cerr<<production_of_harbors[i][j].first<<" "<<production_of_harbors[i][j].second.coords.x<<" "<<production_of_harbors[i][j].second.coords.y<<endl;
        }
    }
}
void vypisComsumption(){
    for(int i=0;i<8;i++){
        cerr<<"konzumacne: "<<endl;
        for(int j=0;j<consumption_of_harbours[i].size();j++){
            cerr<<consumption_of_harbours[i][j].first<<" "<<consumption_of_harbours[i][j].second.coords.x<<" "<<consumption_of_harbours[i][j].second.coords.y<<endl;
        }
    }

}
int get_ship_resources(Ship ship){
    int resources = 0;
    for (int i=0; i<9; i++)
    {
        resources += ship.resources[ResourceEnum(i)];
    }
    return resources;
    
} //sucet vsetkych surovin lode
void Stacionarne(vector<Turn>& turns, Ship ship){
    
    for(auto i:destinations){
        if(i.first==ship.index){
            if(ship.coords == i.second){
                for(int i=0;i<ship_orders.size();i++){
                    if(ship_orders[i].first.index == ship.index){
                        cerr<<"zmena:"<<ship_orders[i].first.index<<endl;
                        ship_orders[i].second.first = 7;
                        return;
                    }
                }
            }
                vector<XY> smery = ziskanieSmery(ship);

            turns.push_back(MoveTurn(ship.index, move_to(ship, i.second, condition,smery)));
            return;
        }
    }
            
                cerr<<occupied_harbors[0].first.coords.x<<" "<<occupied_harbors[0].first.coords.y<<endl;
                    vector<XY> smery = ziskanieSmery(ship);

                turns.push_back(MoveTurn(ship.index, move_to(ship, closest(occupied_harbors[0].first.coords,ship), condition,smery)));
                destinations.push_back(make_pair(ship.index,occupied_harbors[0].first.coords));
                occupied_harbors.erase(occupied_harbors.begin());
                return;
}
*/