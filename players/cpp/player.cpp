#include "common.h"
#include "utils.h"
using namespace std;
World world;

vector<pair<XY,bool>> coords_of_all_harbors;
vector<Harbor> vector_of_found_harbors;
// vector<pair<int,bool>> ocupied_harbors(world.harbors.size(), make_pair(0,false));
vector<pair<int,XY>> ship_orders; //,0=default, 1=kupit, 2=predat, 3=utocit,4=explore
vector<Ship> ourShips;
int tah=0;
bool trebaExplorovat = true;
int indexOfExploringShip = -1;

bool condition(XY a, XY b) { return world.mapa.can_move(b); }
int distance(XY& a, XY& b){ return abs(a.x - b.x) + abs(a.y - b.y); }
bool mamHrbours() { return vector_of_found_harbors.size() == world.harbors.size(); }

int get_ship_resources(Ship ship){
    int resources = 0;
    for (int i=0; i<9; i++)
    {
        resources += ship.resources[ResourceEnum(i)];
    }
    return resources;
    
}

void updateShips(vector<Ship> ships){
    for(Ship ship : world.ships){
        if(ship.mine && ship.is_wreck){
            for(int i=0;i<ships.size();i++){
                if(ships[i].index == ship.index){
                    ships.erase(ships.begin()+i);
                    ship_orders.erase(ship_orders.begin()+i);
                    if(indexOfExploringShip == i) indexOfExploringShip =-1;
                    break;
                }
            }
        }
    }

}



void zijuciExplorer(vector<Turn>& turns){
    if(world.my_ships().size()==0){ return; }
    for(int i=0;i<ship_orders.size();i++){
        if(ship_orders[i].first == 0){
            indexOfExploringShip = i;
            ship_orders[i].first = 4;
            return;
        }
    }

    

    
    cerr<<"explorer: "<<indexOfExploringShip<<endl;
    
    
}

void Explore(Ship ship,vector<Turn>& turns){
    if(world.mapa.tiles[ship.coords.x][ship.coords.y].type != TileEnum::TILE_HARBOR){
            unordered_map<XY, pair<int, XY>> dist;
            vector<XY>& transitions = SMERY;
            vector<Harbor> harbors = world.harbors;
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
        }
        else{
            for(Harbor harbor : world.harbors){
                if(harbor.coords == ship.coords){
                    vector_of_found_harbors.push_back(harbor);
                    for(int i=0;i<coords_of_all_harbors.size();i++){
                        if(coords_of_all_harbors[i].first == harbor.coords){
                            coords_of_all_harbors[i].second = true;
                            break;
                        }
                    }
                    return;
                }
                
            }
            
            return;
        
        }
}



void add_trade_ship_turn(vector<Turn>& turns, Ship ship){
    if(ship.index==ourShips[indexOfExploringShip].index ){
        cerr<<"explorujem"<<endl;
        Explore(ship,turns);
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

    
}


void add_ship_turns(vector<Turn>& turns, vector<Ship> ships){

    for (Ship curr: ships)
    {
        if(curr.stats.ship_class == ShipClass::SHIP_TRADE){
            cerr<<"trade"<<endl;
            add_trade_ship_turn(turns, curr);
        }

        else if (curr.stats.ship_class == ShipClass::SHIP_ATTACK)
        {
           
        }

        else if (curr.stats.ship_class == ShipClass::SHIP_LOOT)
        {

        }
    }

}









vector<Turn> do_turn() {
    vector<Turn> turns;
    if(world.my_ships().size()>ship_orders.size()){
        for(int i=ship_orders.size();i<world.my_ships().size();i++){
            ship_orders.push_back(make_pair(0,world.my_ships()[i].coords));
            ourShips.push_back(world.my_ships()[i]);
        }
    }
    updateShips(world.my_ships());
    if(tah == 1){ // predpocitania na zaciatku hry
        for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(make_pair(harbor.coords,false));
        }
    }
    tah++;
    //predpocitanie

    if(mamHrbours()){ 
    trebaExplorovat = false;
    cerr<<"mam vsetky"<<endl;
     }
    else if(indexOfExploringShip ==-1)
    { zijuciExplorer(turns); }
    //explorovanie

    if (world.my_ships().size() < 2) turns.push_back(BuyTurn(ShipsEnum::Cln));
    //zaciatocne kupovanie lodiek

    add_ship_turns(turns, world.my_ships());
    //pohyb lodi



    
    // for (Ship ship : world.my_ships()) {
    //     if (ship.coords == world.my_base && ship.resources[ResourceEnum::Gold] == 0)
    //         turns.push_back(StoreTurn(ship.index, -5));
    //     else
    //         // takto sa pohybuje smerom na suradnice 50 50
    //         turns.push_back(MoveTurn(ship.index, move_to(ship, {50, 50}, condition)));
    // }
    
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
