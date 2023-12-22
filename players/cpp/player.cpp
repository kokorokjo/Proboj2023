#include "common.h"
#include "utils.h"
using namespace std;
World world;

vector<pair<XY,bool>> coords_of_all_harbors; //bool = ci uz som ho nasiel
vector<Harbor> vector_of_found_harbors; //nasiel som
vector<pair<int,Ship>> ship_orders; //,0=default, 1=predat, 2=kupit, 3=utocit,4=explore
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

void updateShips(vector<Ship> ships){
vector<pair<int,Ship>> new_ship_orders;
bool notin = true;
bool expoler =false;
    for (Ship ship : ships)
    {
        for(int i=0;i<ship_orders.size();i++){
            if(ship_orders[i].second.index == ship.index){
                new_ship_orders.push_back(ship_orders[i]);
                if(ship_orders[i].first == 4)   expoler = true;
                notin = false;
                break;
            }
        }
        if(notin){
            new_ship_orders.push_back(make_pair(0,ship));
        }  
    }
    ship_orders = new_ship_orders;
    if(!expoler) indexOfExploringShip = -1;
} //aktualizacia lodiek

void updateOrders(){
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].first == 0){
            if(ship_orders[i].second.stats.ship_class==ShipClass::SHIP_ATTACK) ship_orders[i].first = 3;
            if(ship_orders[i].second.stats.ship_class==ShipClass::SHIP_TRADE) ship_orders[i].first = 2;   
        }
    }
}

void zijuciExplorer(vector<Turn>& turns){
    if(world.my_ships().size()==0){ return; }
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].first == 0){
            indexOfExploringShip = ship_orders[i].second.index;
            ship_orders[i].first = 4;
            cerr<<"explorer: "<<indexOfExploringShip<<endl;
            return;
        }
    }   
}//nastavi explorera

void umrtvitExplorera(vector<Turn>& turns){
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].first == 4){
            indexOfExploringShip = -1;
            ship_orders[i].first = 2;
            cerr<<"umrtvil som explorera"<<endl;
            return;
        }
    }   
}//zrusim explorera

void Explore(vector<Turn>& turns, Ship ship){
    if(world.mapa.tiles[ship.coords.y][ship.coords.x].type == TileEnum::TILE_HARBOR){
            for(Harbor harbor : world.harbors){
                cerr<<"harbor: "<<harbor.coords.x<<" "<<harbor.coords.y<<endl;
                cerr<<"ship:   "<<ship.coords.x<<" "<<ship.coords.y<<endl;
                if(harbor.coords == ship.coords){
                    vector_of_found_harbors.push_back(harbor);
                    for(int i=0;i<coords_of_all_harbors.size();i++){
                        if(coords_of_all_harbors[i].first == harbor.coords){
                            coords_of_all_harbors[i].second = true;
                            cerr<<"nasiel som:"<<harbor.coords.x<<" "<<harbor.coords.y<<endl;
                            break;
                        }
                    }
                    
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
    //         unordered_map<XY, pair<int, XY>> dist;
    //         vector<XY>& transitions = SMERY;
    //         vector<Harbor> harbors = world.harbors;
    //         bfs(ship.coords, condition, dist, transitions);
    //         XY min_harbor=world.harbors[0].coords;

    // turns.push_back(MoveTurn(ship.index, move_to(ship, min_harbor, condition)));
    // cerr<<"utocim"<<ship.coords.x<<" "<<ship.coords.y<<endl;

} //pohyb utocnej lode

void Buy(vector<Turn>& turns, Ship ship){
    

}//kupovanie resourcov

void Sell(){

}//predavanie resourcov



void add_trade_ship_turn(vector<Turn>& turns, Ship ship){
    if(ship.index==indexOfExploringShip ){
        cerr<<"explorujem"<<endl;
        Explore(turns,ship);
        return;

    }
    // if(get_ship_resources(ship) == 0){
    //     //nemam nic

    //     if(ship.coords == world.my_base){
    //         turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo, world.gold)));
    //         return;
    //     }
    //     turns.push_back(MoveTurn(ship.index, move_to(ship, world.my_base, condition)));
    //     return;
    // }


    // for (Ship ship : world.my_ships()) {
    //     if (ship.coords == world.my_base && ship.resources[ResourceEnum::Gold] == 0)
    //         turns.push_back(StoreTurn(ship.index, -5));
    // }

    
} //pohyb obchodnej lode







void add_ship_turns(vector<Turn>& turns, vector<Ship> ships){

    for (int i=0;i<ships.size();i++)
    {
        switch (ship_orders[i].first)
        {
            case 4:
                cerr<<"explorujem"<<endl;
                Explore(turns,ship_orders[i].second);
                break;
            case 3:
                Attack(turns, ship_orders[i].second);
                break;
            case 2:
                Buy(turns, ship_orders[i].second);
                break;
            case 1:
                Sell();
                break;
            
        }
    }

}//pohyb lodi









vector<Turn> do_turn() {
    vector<Turn> turns;
    updateShips(world.my_ships());
    if(tah == 1) allHarbours();
    tah++;
    //predpocitanie

    if(mamHrbours()){ 
    trebaExplorovat = false;
    cerr<<"mam vsetky"<<endl;
    umrtvitExplorera(turns);
    }
    else if(indexOfExploringShip ==-1){ 
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
