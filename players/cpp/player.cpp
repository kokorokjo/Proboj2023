#include "common.h"
#include "utils.h"
using namespace std;
World world;

struct Trade{
    double odhad;
    // Harbor FromH;
    // Harbor ToH;
    int resource;

    
} trade;


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

bool condition(XY a, XY b) {
    if(!world.mapa.can_move(b)){
        return false;
    }
    for(Ship ship : world.ships){
        if(ship.coords == b){
            return false;
        }
    
    }
    return true;
}//ci sa mozem pohnut na dany bod

void allHarbours(){
    for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(make_pair(harbor.coords,false));
            cerr<<"Zharbor: "<<harbor.coords.y<<" "<<harbor.coords.x<<" - "<<endl;
        }
} //da vsetky pristavy do vectora coords_of_all_harbors

int get_ship_resources(Ship ship){
    int resources = 0;
    for (int i=0; i<9; i++)
    {
        resources += ship.resources[ResourceEnum(i)];
    }
    return resources;
    
} //sucet vsetkych surovin lode

void insertProduction(){

}

void updateShips(vector<Ship> ships){
vector<pair<Ship,pair<int,Trade>>> new_ship_orders;
bool notin = true;
bool expoler =false;
    for (Ship ship : ships)
    {
        for(int i=0;i<ship_orders.size();i++){
            if(ship_orders[i].first.index == ship.index){
                new_ship_orders.push_back(ship_orders[i]);
                if(ship_orders[i].second.first == 4)   expoler = true;
                notin = false;
                break;
            }
        }
        if(notin){
            new_ship_orders.push_back(make_pair(ship,make_pair(0,trade)));
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

void zijuciExplorer(vector<Turn>& turns){
    if(world.my_ships().size()==0){ return; }
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
            cerr<<"umrtvil som explorera"<<endl;
            return;
        }
    }   
}//zrusim explorera

void acquireGold(vector<Turn>& turns, Ship ship){
    if(ship.coords == world.my_base){
        turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo/2, world.gold)));
        return;
    }
    turns.push_back(MoveTurn(ship.index, move_to(ship, world.my_base, condition)));
    return;

}


void Explore(vector<Turn>& turns, Ship ship){
    if(world.mapa.tiles[ship.coords.y][ship.coords.x].type == TileEnum::TILE_HARBOR){
            cerr<<"ship:   "<<ship.coords.x<<" "<<ship.coords.y<<endl;
            for(Harbor harbor : world.harbors){
                cerr<<"harbor: "<<harbor.coords.x<<" "<<harbor.coords.y<<endl;
                if(harbor.coords == ship.coords){
                    vector_of_found_harbors.push_back(harbor);
                    for(int i=0;i<8;i++){
                        if(harbor.production.resources[i]>0){
                            production_of_harbors[i].push_back(make_pair(harbor.production.resources[i],harbor));

                        }

                        if(harbor.production.resources[i]<0){
                            consumption_of_harbours[i].push_back(make_pair(harbor.production.resources[i],harbor));
                        }
                    }//zapisem si produkciu a konzumaciu pristavov
                    for(int i=0;i<coords_of_all_harbors.size();i++){
                        if(coords_of_all_harbors[i].first == harbor.coords){
                            coords_of_all_harbors[i].second = true;
                            cerr<<"nasiel som:"<<harbor.coords.x<<" "<<harbor.coords.y<<endl;
                            cerr<<"ship:   "<<ship.coords.x<<" "<<ship.coords.y<<endl;
                            break;
                        }
                    }//oznacim si ze som ho nasel
                    
                }
                
            }
            
            
        
        }//ak sa nahcadzam na pristave tak si ho zapisem
    
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

            turns.push_back(MoveTurn(ship.index, move_to(ship, min_harbor, condition)));
            return;
        
}// najdem najblizsi pristav a pohnem sa na neho

void Attack(vector<Turn>& turns, Ship ship){
} //pohyb utocnej lode

void Calculate(vector<Turn>& turns, Ship ship){
    if(ship.resources[ResourceEnum::Gold] == 0) acquireGold(turns,ship);
    else{

    }
}

void Buy(vector<Turn>& turns, Ship ship){
    
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
                cerr<<"explorujem"<<endl;
                Explore(turns,curr);
                break;
            case 3:
                Attack(turns,curr);
                break;
            case 2:
                Buy(turns,curr);
                break;
            case 1:
                Sell();
                break;
            case 5:
                Calculate(turns,curr);
                break;
            
        }
        
        i++;
    }

}//pohyb lodi









vector<Turn> do_turn() {
    vector<Turn> turns;
    updateShips(world.my_ships());
    if(tah == 1) allHarbours();
    tah++;
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
    cerr << "Takto mozete vypisovat do logov" << endl;
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
