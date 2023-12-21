#include "common.h"
#include "utils.h"
using namespace std;

World world;
vector<pair<ShipsEnum,bool>> ships_to_buy = {{ShipsEnum::Cln,false},
                                            {ShipsEnum::Cln,false},
                                            {ShipsEnum::SomalianPirateShip,false}};

map<ShipsEnum, int> ship_prizes = { {ShipsEnum::Cln, 10},
                                    {ShipsEnum::Plt, 30},
                                    {ShipsEnum::SmallMerchantShip, 100},
                                    {ShipsEnum::LargeMerchantShip, 200},
                                    {ShipsEnum::SomalianPirateShip, 15},
                                    {ShipsEnum::BlackPearl, 50},
                                    {ShipsEnum::SniperAttackShip, 30},
                                    {ShipsEnum::LooterScooter, 50}};

map<XY, Harbor> found_harbors;
vector<Harbor> vector_of_found_harbors;
vector<pair<int,XY>> ship_orders; //0=default, 1=kupit, 2=predat, 3=utocit,
vector<XY> coords_of_all_harbors;
vector<pair<int,bool>> ocupied_harbors(world.harbors.size(), make_pair(0,false));


bool condition(XY a, XY b) { return world.mapa.can_move(b); }

// bool condition_for_attack_ship(XY a, XY b){
//     if(!world.mapa.can_move(b)){
//         return false;
//     }

//     for(XY harbor: coords_of_all_harbors){
//         if(distance(b, harbor) <= 2){ //TODO neskor upravit minimalnu vzdialenost
//             return false;
//         }
//     }

//     return true;
// }

int tah = 0;

int get_ship_resources(Ship ship){
    int resources = 0;
    for (int i=0; i<9; i++)
    {
        resources += ship.resources[ResourceEnum(i)];
    }
    return resources;
    
}
void add_buy_turns(vector<Turn>& turns, int gold_to_use)
{

    for (int i = 0; i < ships_to_buy.size(); i++)
    {
        if(!ships_to_buy[i].second && ship_prizes[ships_to_buy[i].first] <= gold_to_use){
            ships_to_buy[i].second = true;
            turns.push_back(BuyTurn(ships_to_buy[i].first));
        }
    }
    
}
pair<Harbor, ResourceEnum> find_best_harbor(Harbor harbor){
    
    int best_i = 0;
    int best_j = 0;
    int best_diff = 0;


    for(int i=0;i<vector_of_found_harbors.size();i++){
        for(int j = 0; j < 9; j++)
        {
            int diff = harbor.production[ResourceEnum(j)] - vector_of_found_harbors[i].production[ResourceEnum(j)];
            if(diff > best_diff){
                best_i = i;
                best_j = j;
                best_diff = diff;
            }
        }
    }

    cerr<<"best i"<<best_i<<endl;
    cerr<<"best j"<<best_j<<endl;
    cerr<<"best diff"<<best_diff<<endl;
    cerr<<"harborproduct"<<harbor.production[ResourceEnum(best_j)]<<endl;
    cerr<<"vectoir of"<<vector_of_found_harbors[best_i].production[ResourceEnum(best_j)]<<endl;
    return make_pair(vector_of_found_harbors[best_i], ResourceEnum(best_j));

}

void add_trade_ship_turn(vector<Turn>& turns, Ship ship){
    if(get_ship_resources(ship) == 0){
        //nemam peniaze

        if(ship.coords == world.my_base){
            turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo, world.gold)));
            return;
        }
        turns.push_back(MoveTurn(ship.index, move_to(ship, world.my_base, condition)));
        return;
    }

    else if(get_ship_resources(ship)-ship.resources[ResourceEnum::Gold] != 0){
        ocupied_harbors[ship.index].second = false;
        XY temp_coords=ship.coords;
        int resources_to_sell ;
        for(int i=0;i<vector_of_found_harbors.size();i++){
            int profit=1000;
            

            for(int j=0;j<9;j++){
                for(int i=0;i<vector_of_found_harbors.size();i++){
                    if(vector_of_found_harbors[i].production[ResourceEnum(j)]<profit){
                        profit=vector_of_found_harbors[i].production[ResourceEnum(j)];
                        temp_coords=vector_of_found_harbors[i].coords;
                        resources_to_sell=j;
                    }
                }
            }
        }
        if(temp_coords!=ship.coords){
            turns.push_back(MoveTurn(ship.index, move_to(ship, temp_coords, condition)));
            return;
        }
        else{

            turns.push_back(TradeTurn(ship.index,ResourceEnum(resources_to_sell),-world.my_ships()[ship.index].resources[ResourceEnum(resources_to_sell)]));
            return;
        }

    }

    else{
        //mam iba peniaze
        if(world.mapa.tiles[ship.coords.y][ship.coords.x].type != TileEnum::TILE_HARBOR){


            //nie som v pristave a idem do pristavu kupit
            unordered_map<XY, std::pair<int, XY>> dist;
            vector<XY>& transitions = SMERY;
            vector<Harbor> harbors = world.harbors;
            int index_of_harbor = 0;
            bfs(ship.coords, condition, dist, transitions);
            int min_dist = 1000000;
            XY min_harbor;
            for(int i=0;i<harbors.size();i++){
                if(ocupied_harbors[i].second && ocupied_harbors[i].first!=ship.index){
                    continue;
                }
                if(dist[harbors[i].coords].first < min_dist){
                    min_dist = dist[harbors[i].coords].first;
                    min_harbor = harbors[i].coords;
                    // cerr<<" nieco"<<min_harbor.x<<" "<<min_harbor.y<<endl;
                    index_of_harbor = i;
                }
                
            }
            

            ocupied_harbors[index_of_harbor].first = ship.index;
            ocupied_harbors[index_of_harbor].second = true;
            turns.push_back(MoveTurn(ship.index, move_to(ship, min_harbor, condition)));
            ship_orders[ship.index].first = 1;
            ship_orders[ship.index].second = min_harbor;
            
            return;


        }

        else{

            

            //mam peniaze alod v pristave a idem kupit goods
            cerr<<"som v pristave"<<endl;
            if(vector_of_found_harbors.size()>=2){

            int index_of_harbor = 0;
            ResourceEnum best_resource = ResourceEnum::Gold;
            Harbor best_harbor = world.harbors[0];
            for(Harbor harbor : world.harbors){
                if(harbor.coords == ship.coords){
                    cerr<<"som"<<endl;
                    best_resource = find_best_harbor(harbor).second;
                    best_harbor = find_best_harbor(harbor).first;
                    // if(found_harbors.find(harbor.coords) == found_harbors.end()){}
                    cerr<<"nejsom"<<endl;


                    break;
                }
            }
            int resources_to_store = min(ship.resources[ResourceEnum::Gold]/best_harbor.resource_cost(best_resource), ship.stats.max_cargo);
            cerr<<"resources to store"<<resources_to_store<<endl;
            cerr<<"best resource"<<best_harbor.resource_cost(best_resource)<<endl;
            cerr<<"neviem"<<ship.resources[ResourceEnum::Gold]<<endl;
            cerr<<"ship resources"<<ship.stats.max_cargo<<endl;
            if(resources_to_store != 0){
                turns.push_back(TradeTurn(ship.index, best_resource,ship.resources[ResourceEnum::Gold]/resources_to_store));
            }
            
            
            
            ship_orders[ship.index].first = 2;
            ship_orders[ship.index].second = best_harbor.coords;
            return;
            }
            else{
                bool bol=false;
                bool som=false;

                for(int i=0;i<world.harbors.size();i++){
                    if(world.harbors[i].coords == ship.coords){
                        som=true;
                        for(int j=0;j<vector_of_found_harbors.size();j++){
                            if(vector_of_found_harbors[j].coords==world.harbors[i].coords){
                                bol=true;
                                break;
                                
                            }
                        }
                        if(!bol){
                            vector_of_found_harbors.push_back(world.harbors[i]);
                        }
                    }
                    if(som){
                        break;
                    }
                        
                }
            }
        
        }

    }
}

// pair<Ship, bool> enemy_ship_nearby(XY temp_coords, int range){
//     for(Ship ship : world.ships){
//         if(!ship.mine && distance(temp_coords, ship.coords)  <= range){
//             return make_pair(ship, true);
//         }
//     }
//     return make_pair(Ship(), false);
// }

// void add_attack_ship_turn(vector<Turn>& turns, Ship ship){
//     //TODO co ma robit attack ship

//     pair<Ship, bool> enemy_ship = enemy_ship_nearby(ship.coords, ship.stats.range);

//     if(enemy_ship.second){
//         turns.push_back(ShootTurn(ship.index, enemy_ship.first.index));
//         return;
//     }

//     unordered_map<XY, std::pair<int, XY>> dist;
//     vector<Harbor> harbors = world.harbors;
//     bfs(ship.coords, condition, dist); //TODO upravit condition

//     int min_dist = 1000000;
//     XY closest_ship;

//     for(Ship curr: world.ships){
//         if(curr.mine){
//             if(dist[curr.coords].first < min_dist){
//                 min_dist = dist[curr.coords].first;
//                 closest_ship = curr.coords;
//             }
//         }
//     }

//     turns.push_back(MoveTurn(ship.index, move_to(ship, closest_ship, condition)));
// }

void add_loot_ship_turn(vector<Turn>& turns, Ship ship){
    //TODO co ma robit loot ship
}


void add_ship_turns(vector<Turn>& turns, vector<Ship> ships){

    for (Ship curr: ships)
    {
        if(curr.stats.ship_class == ShipClass::SHIP_TRADE){
            add_trade_ship_turn(turns, curr);
        }

        else if (curr.stats.ship_class == ShipClass::SHIP_ATTACK)
        {
            // add_attack_ship_turn(turns, curr);
        }

        else if (curr.stats.ship_class == ShipClass::SHIP_LOOT)
        {
            add_loot_ship_turn(turns, curr);
        }
    }

}



vector<Turn> do_turn() {
    tah++;
    if(tah == 1){ // predpocitania na zaciatku hry
        for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(harbor.coords);
        }
    }

    //TODO upavit tuto funkciu

    if(world.my_ships().size()>ship_orders.size()){
        for(int i=ship_orders.size();i<world.my_ships().size();i++){
            ship_orders.push_back(make_pair(0,world.my_ships()[i].coords));
        }
    }
    if(ocupied_harbors.size() == 0){
                ocupied_harbors.resize(world.harbors.size(), make_pair(0,false));
    }
    // sem ide vas kod
    vector<Turn> turns;

    int gold_to_use = world.gold / 2;

    

    add_buy_turns(turns, gold_to_use);
    add_ship_turns(turns, world.my_ships());


    cerr << "Takto mozete vypisovat do logov" << endl;
    cerr << turns << endl;
    return turns;
}









int distance(XY& a, XY& b){
    return abs(a.x - b.x) + abs(a.y - b.y);
}


ShipsEnum get_ship_enum(Ship ship){
    ShipClass sclass = ship.stats.ship_class;

    if (sclass == ShipClass::SHIP_ATTACK){
        if(ship.stats.price == 15){
            return ShipsEnum::SomalianPirateShip;
        }
        else if (ship.stats.price == 30)
        {
            return ShipsEnum::SniperAttackShip;
        }
        else if (ship.stats.price == 50)
        {
            return ShipsEnum::BlackPearl;
        }
        
    }
    else if (sclass == ShipClass::SHIP_TRADE)
    {
        if(ship.stats.price == 10){
            return ShipsEnum::Cln;
        }
        else if (ship.stats.price ==  30)
        {
            return ShipsEnum::Plt;
        }
        else if (ship.stats.price == 50)
        {
            return ShipsEnum::LargeMerchantShip;
        }
    }
    else if (sclass == ShipClass::SHIP_LOOT)
    {
        return ShipsEnum::LooterScooter;
    }
    
    
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
