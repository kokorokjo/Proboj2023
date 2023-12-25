#include "common.h"
#include "utils.h"
using namespace std;
World world;

vector<ResourceEnum> RESOURCES = { ResourceEnum::Wood, ResourceEnum::Stone, ResourceEnum::Iron, ResourceEnum::Gem, ResourceEnum::Wool, ResourceEnum::Hide, ResourceEnum::Wheat, ResourceEnum::Pineapple, ResourceEnum::Gold };



struct Trade{
    double odhad;
    Harbor FromH;
    Harbor ToH;
    int tovar;

    
}t;

vector<vector<int>> mapka,mapkaD;

vector<pair<XY,bool>> coords_of_all_harbors; //bool = ci uz som ho nasiel
vector<Harbor> vector_of_found_harbors; //nasiel som
vector<vector<pair<int,Harbor>>> production_of_harbors(8),consumption_of_harbours(8); //produkcia pristavov, konzumacia pristavov
vector<pair<Ship,pair<int,Trade>>> ship_orders; //,0=default, 1=predat, 2=kupit, 3=utocit,4=explore,5=calculate
vector<Trade> trades; //vsetky trades

int tah=0; //pocitam si tahy
bool trebaExplorovat = true; //ci treba explorovat
int indexOfExploringShip = -1; //index lode ktora exploruje

int distance(XY& a, XY& b){ return abs(a.x - b.x) + abs(a.y - b.y); } //vzdialenost dvoch bodov
bool mamHrbours() { return vector_of_found_harbors.size() == world.harbors.size(); } //ci mam vsetky pristavy

int manyToBuy(pair<Ship,pair<int,Trade>> s,ResourceEnum res,int pocet){
    int cost=s.second.second.FromH.resource_cost(res);
    int maximalnyPocet=s.first.resources.resources[8]/cost;
    cerr<<"cost: "<<cost<<" pocet: "<<pocet<<" maximalnyPocet: "<<maximalnyPocet<<" resources: "<<s.first.resources.resources[8]<<endl;
    if(pocet>maximalnyPocet) return pocet-1;
    return manyToBuy(s,res,pocet+1);


}

bool condition(XY a, XY b) {
    if(b.x < 0 || b.x >= world.mapa.width || b.y < 0 || b.y >= world.mapa.height) return false;
    if(mapka[b.y][b.x]==2) return false;
    return true;
}//ci sa mozem pohnut na dany bod

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

XY closest(XY destination,Ship ship){
    if(ship.coords == destination) return destination;
    while(true){
        int vzdialenost = 1;
        if(mapka[destination.y][destination.x]==0) return destination;
        vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, destination.x, destination.y, vzdialenost);
        for(vector<int> bod : body){
            if(mapka[bod[1]][bod[0]]!=2) return XY(bod[0],bod[1]);
        }

    }
}//najblizsi bod kam pohnut

int get_ship_resources(Ship ship){
    int resources = 0;
    for (int i=0; i<9; i++)
    {
        resources += ship.resources[ResourceEnum(i)];
    }
    return resources;
    
} //sucet vsetkych surovin lode

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
}

void makeTradeCons(int tovar,Harbor harbor){
    cerr<<"makeTradeCons"<<endl;
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

}

void allHarbours(){
    for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(make_pair(harbor.coords,false));
            cerr<<"Zharbor: "<<harbor.coords.y<<" "<<harbor.coords.x<<" - "<<endl;
        }
} //da vsetky pristavy do vectora coords_of_all_harbors

void updateShips(vector<Ship> ships){
vector<pair<Ship,pair<int,Trade>>> new_ship_orders;
bool notin = true;
bool expoler =false;
    for (Ship ship : ships)
    {
        notin = true;
        for(int i=0;i<ship_orders.size();i++){
            if(ship_orders[i].first.index == ship.index){
                ship_orders[i].first = ship;
                new_ship_orders.push_back(ship_orders[i]);
                if(ship_orders[i].second.first == 4)   expoler = true;
                notin = false;
                break;
            }
        }
        if(notin){
            cerr<<"new ship: "<<ship.index<<endl;
            new_ship_orders.push_back(make_pair(ship,make_pair(0,t)));
        }  
    }
    ship_orders = new_ship_orders;
    if(!expoler) indexOfExploringShip = -1;
} //aktualizacia lodiek

void updateOrders(){
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 0){
            if(ship_orders[i].first.stats.ship_class==ShipClass::SHIP_ATTACK) ship_orders[i].second.first = 3;
            if(ship_orders[i].first.stats.ship_class==ShipClass::SHIP_TRADE) ship_orders[i].second.first = 5;   
        }
    }
}

void createMap(){
    mapkaD.resize(world.mapa.height,vector<int> (world.mapa.width,0));
    mapka.resize(world.mapa.height,vector<int> (world.mapa.width,0));

    for(int i=0;i<world.mapa.height;i++){
        for(int j=0;j<world.mapa.width;j++){
            if(world.mapa.tiles[i][j].type==TileEnum::TILE_BASE){                
                if(world.my_base!=XY(j,i)){  
                    vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, j, i, 4);
                    for (vector<int> bod : body) {
                        if(mapkaD[bod[1]][bod[0]]!=1&&mapkaD[bod[1]][bod[0]]!=3)
                        mapkaD[bod[1]][bod[0]]=2;
                        }
                        mapkaD[i][j]=3;
                }
                
            }//base 3 a okolie 2
            else if(world.mapa.tiles[i][j].type==TileEnum::TILE_GROUND){
                mapkaD[i][j]=2;
                

            }//zem 2
            else if(world.mapa.tiles[i][j].type==TileEnum::TILE_HARBOR){
                mapkaD[i][j]=1;
                    vector<vector<int>> body = manhattanBody(world.mapa.height,world.mapa.width, j, i, 8);
                    for (vector<int> bod : body) {
                        if(mapkaD[bod[1]][bod[0]]==0)
                        mapkaD[bod[1]][bod[0]]=4;
                        }
                        mapkaD[i][j]=1;

            }//harbor 1 a okolie  4
            
        }
        
    }
    for(auto i:mapkaD){
        for(auto j:i){
            cerr<<j;
        }
        cerr<<endl;
    }//vypisem si mapku
}//vytvorim si mapkuD
void updateMap(){
    mapka=mapkaD;
    for(Ship ship:world.ships){
        if(mapka[ship.coords.y][ship.coords.x]==0||mapka[ship.coords.y][ship.coords.x]==4)
        mapka[ship.coords.y][ship.coords.x]=2;
    }
}

void zijuciExplorer(vector<Turn>& turns){
    if(world.my_ships().size()==0) return;
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 0){
            indexOfExploringShip = ship_orders[i].first.index;
            ship_orders[i].second.first = 4;
            cerr<<"explorer: "<<indexOfExploringShip<<endl;
            return;
        }
    }   
}//nastavi explorera

void umrtvitExplorera(vector<Turn>& turns){
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].second.first == 4){
            indexOfExploringShip = -1;
            ship_orders[i].second.first = 5;
            cerr<<"umrtvil som explorera"<<ship_orders[i].first.index<<endl;
            return;
        }
    }   
}//zrusim explorera

void acquireGold(vector<Turn>& turns, Ship ship){
    cerr<<"acquire gold"<<endl;
    if(ship.coords == world.my_base){
        turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo, world.gold)));
        return;
    }
    turns.push_back(MoveTurn(ship.index, move_to(ship, closest(world.my_base,ship), condition)));
    return;

}


void Explore(vector<Turn>& turns, Ship ship){
    if(world.mapa.tiles[ship.coords.y][ship.coords.x].type == TileEnum::TILE_HARBOR){
            for(Harbor harbor : world.harbors){
                if(harbor.coords == ship.coords){
                    vector_of_found_harbors.push_back(harbor);
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
            if(mamHrbours())return;
            cerr<<"ship:"<<ship.coords.x<<" "<<ship.coords.y<<endl;
            unordered_map<XY, pair<int, XY>> dist;
            vector<XY>& transitions = SMERY;
            vector<Harbor> harbors = world.harbors;
            bfs(ship.coords, condition, dist, transitions);
            int min_dist = 1000000;
            XY min_harbor;
            for(int i=0;i<harbors.size();i++){
                if(dist[harbors[i].coords].first < min_dist&&coords_of_all_harbors[i].second==false){
                    min_dist = dist[harbors[i].coords].first;
                    min_harbor = harbors[i].coords;
                }
                
            }

            turns.push_back(MoveTurn(ship.index, move_to(ship, closest(min_harbor,ship), condition)));
            return;
        
}// najdem najblizsi pristav a pohnem sa na neho

void Attack(vector<Turn>& turns, Ship ship){
} //pohyb utocnej lode

void Calculate(vector<Turn>& turns, Ship ship){
    if(ship.resources.resources[8] == 0) acquireGold(turns, ship);
    else{
        for(int i=0;i<ship_orders.size();i++){
            if(ship_orders[i].first.index == ship.index){
                if(trades.size()==0) Explore(turns,ship);
                else{
                    ship_orders[i].second.first = 2;
                    ship_orders[i].second.second=trades[0];
                    cerr<<"calculate: "<<ship_orders[i].second.second.FromH.coords.x<<" "<<ship_orders[i].second.second.FromH.coords.y<<endl;
                    cerr<<"nieci: "<<trades[0].FromH.coords.x<<" "<<trades[0].FromH.coords.y<<endl;
                    turns.push_back(MoveTurn(ship.index, move_to(ship, closest(trades[0].FromH.coords,ship), condition)));
                }

                break;
            }
        }
    }
    return;
    

}

void Buy(vector<Turn>& turns, Ship ship){
    int f=0;
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
        turns.push_back(MoveTurn(ship.index, move_to(ship, closest(i.second.second.FromH.coords,ship), condition)));
        return;
        }
    }
    f++;
    }
}//kupovanie resourcov

void Sell(){

}//predavanie resourcov


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





void add_ship_turns(vector<Turn>& turns, vector<Ship> ships){
    int i=0;
    for(Ship curr : ships){
        switch (ship_orders[i].second.first)
        {
            case 4:
                cerr<<"explorujem"<<curr.index<<endl;
                Explore(turns,curr);
                break;
            case 3:
                cerr<<"utocim"<<endl;
                Attack(turns,curr);
                break;
            case 2:
                cerr<<"kupim"<<endl;
                Buy(turns,curr);
                break;
            case 1:
                cerr<<"predam"<<endl;
                Sell();
                break;
            case 5:
                cerr<<"pocitam"<<curr.index<<endl;
                Calculate(turns,curr);
                break;
            default:
                cerr<<"nic"<<endl;
                break;
            
        }
        
        i++;
    }

}//pohyb lodi









vector<Turn> do_turn() {
    vector<Turn> turns;
    updateShips(world.my_ships());
    if(tah == 1) {
        allHarbours();
        createMap();
        }
    tah++;
    updateMap();
    //predpocitanie

    if(mamHrbours()&&trebaExplorovat){ 
    trebaExplorovat = false;
    cerr<<"mam vsetky"<<endl;
    umrtvitExplorera(turns);
    vypisProduction();
    vypisComsumption();
    }
    else if(indexOfExploringShip ==-1&&trebaExplorovat){ 
    zijuciExplorer(turns);
    }
    //explorovanie

    if (world.my_ships().size() < 2) turns.push_back(BuyTurn(ShipsEnum::Cln));
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
