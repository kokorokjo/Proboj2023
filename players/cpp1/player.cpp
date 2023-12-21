#include "common.h"
#include "utils.h"
using namespace std;

World world;
vector<pair<ShipsEnum,bool>> ships_to_buy = {{ShipsEnum::Cln,false},
                                            {ShipsEnum::Cln,false},
                                            {ShipsEnum::SomalianPirateShip,false},
                                            {ShipsEnum::Cln,false},
                                            {ShipsEnum::SomalianPirateShip,false},
                                            {ShipsEnum::Plt,false},
                                            {ShipsEnum::SomalianPirateShip,false},
                                            {ShipsEnum::Plt,false},
                                            {ShipsEnum::BlackPearl,false},
                                            {ShipsEnum::Plt,false},
                                            {ShipsEnum::BlackPearl,false},
                                            {ShipsEnum::SmallMerchantShip,false},};

map<ShipsEnum, int> ship_prizes = { {ShipsEnum::Cln, 10},
                                    {ShipsEnum::Plt, 30},
                                    {ShipsEnum::SmallMerchantShip, 100},
                                    {ShipsEnum::LargeMerchantShip, 200},
                                    {ShipsEnum::SomalianPirateShip, 15},
                                    {ShipsEnum::BlackPearl, 50},
                                    {ShipsEnum::SniperAttackShip, 30},
                                    {ShipsEnum::LooterScooter, 50}};

map<XY, Harbor> found_harbors; //tot neviem kde to pouzit
vector<Harbor> vector_of_found_harbors;
vector<XY> coords_of_all_harbors;
vector<pair<XY,int>> occupied_harbors;

int no_of_explorers = 0;

enum class TaskEnum: int{
    GET_GOLD,
    DROP_GOLD,
    BUY_RESOURCES,
    SELL_RESOURCES,
    EXPLORE,
    FOLLOW,
    NONE
};

struct Task{
    //uloha ktoru ma splnit ship
    TaskEnum task;

    //suradnice kam ma ist
    XY coords;
};

vector<pair<Ship, Task>> our_ships;
unordered_set<int> our_ships_ids;

int tah = 0;


void update_vector_of_ships(vector<pair<Ship, Task>>& ships, unordered_set<int>& ids){
    for(Ship ship : world.ships){
        if(ship.mine && ids.find(ship.index) == ids.end()){
            if(ship.stats.ship_class == ShipClass::SHIP_TRADE){
            
                if(no_of_explorers == 0){
                    ships.push_back(make_pair(ship, Task{TaskEnum::EXPLORE, XY(0,0)}));
                    ids.insert(ship.index);
                    no_of_explorers++;
                }
                else{
                    ships.push_back(make_pair(ship, Task{TaskEnum::GET_GOLD, world.my_base}));
                    ids.insert(ship.index);
                }
            }
            else if (ship.stats.ship_class == ShipClass::SHIP_ATTACK)
            {
                ships.push_back(make_pair(ship, Task{TaskEnum::FOLLOW, XY(0,0)}));
                ids.insert(ship.index);
            }
            else if (ship.stats.ship_class == ShipClass::SHIP_LOOT)
            {
                ships.push_back(make_pair(ship, Task{TaskEnum::NONE, XY(0,0)}));
                ids.insert(ship.index);
            }
        }
        else if (ship.mine && ship.is_wreck)
        {
            for(int i=0;i<ships.size();i++){
                if(ships[i].first.index == ship.index){
                    ships.erase(ships.begin()+i);
                    ids.erase(ship.index);
                    break;
                }
            }
        }
        
    }
}

void assign_task(vector<pair<Ship, Task>>& ships, int ship_id, Task task){
    
    for(int i=0;i<ships.size();i++){
        if(ships[i].first.index == ship_id){
            ships[i].second = task;
            return;
        }
    }
    // ships.push_back(make_pair(world.ships[ship_id], task));
}


bool get_gold(Ship ship){
    if(ship.resources[ResourceEnum::Gold] == ship.stats.max_cargo/2){
        return true;
    }
}
bool drop_gold(Ship ship){
    if(ship.resources[ResourceEnum::Gold] == ship.stats.max_cargo/2){
        return true;
    }
}
bool buy_resources(Ship ship, int material ){
    if(ship.resources[ResourceEnum(material)] != 0){
        return true;
    }
}
bool sell_resources(Ship ship, int material){
    if(ship.resources[ResourceEnum(material)] == 0){
        return true;
    }
}
bool explore(Ship ship){
    if(vector_of_found_harbors.size() == world.harbors.size()){
        return true;
    }
}

void give_new_task(vector<pair<Ship, Task>>& ships,int index,int material){
    if(ships[index].second.task == TaskEnum::GET_GOLD){
        if(get_gold(ships[index].first)){
            assign_task(ships, index, {TaskEnum::BUY_RESOURCES, XY(0,0)});
        }

    }
    else if (ships[index].second.task==TaskEnum::BUY_RESOURCES){
        if(buy_resources(ships[index].first, material)){
            assign_task(ships, index, {TaskEnum::SELL_RESOURCES, XY(0,0)});
        }
    }
    else if(ships[index].second.task == TaskEnum::SELL_RESOURCES){ 
        if(sell_resources(ships[index].first, material)){
            assign_task(ships, index, {TaskEnum::DROP_GOLD, XY(0,0)});
        }


    }
    else if(ships[index].second.task == TaskEnum::DROP_GOLD){
        if(drop_gold(ships[index].first)){
            assign_task(ships, index, {TaskEnum::GET_GOLD, XY(0,0)});
        }
    
        
    }
    else if(ships[index].second.task == TaskEnum::EXPLORE){
        if(explore(ships[index].first)){
            assign_task(ships, index, {TaskEnum::GET_GOLD, XY(0,0)});
        }
    
    }
        
}






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
}

bool condition_for_attack_ship(XY a, XY b){
    if(!world.mapa.can_move(b)){
        return false;
    }

    for(Ship ship : world.ships){
        if(ship.coords == b){
            return false;
        }
    
    }

    for(XY harbor: coords_of_all_harbors){
        if(abs(harbor.x - b.x) + abs(harbor.y - b.y) <= 5){ //TODO neskor upravit minimalnu vzdialenost
            return false;
        }
    }

    return true;
}

void add_to_occupant(XY coords, int index){
    bool found = false;
    for(pair<XY,int> pair : occupied_harbors){
        if(pair.first == coords&&pair.second!=index){
            found = true;
            break;
        }
    }
    if(!found){
        occupied_harbors.push_back(make_pair(coords, index));
    }
}
void vector_harbor_instert(Harbor harbor){
    bool found = false;
    for(Harbor har : vector_of_found_harbors){
        if(har.coords == harbor.coords){
            found = true;
            break;
        }
    }
    if(!found){
        vector_of_found_harbors.push_back(harbor);
    }
}
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
            turns.push_back(BuyTurn(ships_to_buy[i].first));
            ships_to_buy[i].second = true;
            gold_to_use -= ship_prizes[ships_to_buy[i].first];
        }
    }
    
}

pair<Harbor, ResourceEnum> find_best_harbor(Harbor harbor){
    
    int best_i = 0;
    int best_j = 0;
    int best_diff = 0;
    bool found = false;

    for(int i=0;i<vector_of_found_harbors.size();i++){
        for(int j = 0; j < 8; j++) // gold sa nepredava
        {
            if(vector_of_found_harbors[i].production[ResourceEnum(j)] >= 0){ //ak neprodukuje dany resource tak ho preskoc
                continue;
            }
            int diff = harbor.production[ResourceEnum(j)] - vector_of_found_harbors[i].production[ResourceEnum(j)];
            
            if(diff > best_diff){
                best_i = i;
                best_j = j;
                best_diff = diff;
                found = true;
            }
        }
    }

    if(!found){
        return make_pair(vector_of_found_harbors[0], ResourceEnum::Gold);
    }

    return make_pair(vector_of_found_harbors[best_i], ResourceEnum(best_j));

}

void add_trade_ship_turn(vector<Turn>& turns, pair<Ship, Task> ship_task){
    Ship ship = ship_task.first;
    Task task = ship_task.second;

    if(task.task == TaskEnum::EXPLORE){

        if(world.mapa.tiles[ship.coords.x][ship.coords.y].type != TileEnum::TILE_HARBOR){
            unordered_map<XY, std::pair<int, XY>> dist;
            vector<XY>& transitions = SMERY;
            vector<Harbor> harbors = world.harbors;
            bfs(ship.coords, condition, dist, transitions);
            int min_dist = 1000000;
            XY min_harbor;
            for(int i=0;i<harbors.size();i++){
                if(dist[harbors[i].coords].first < min_dist){
                    min_dist = dist[harbors[i].coords].first;
                    min_harbor = harbors[i].coords;
                }
                
            }
            turns.push_back(MoveTurn(ship.index, move_to(ship, min_harbor, condition)));
            // ship_orders[ship.index].first = 1;
            // ship_orders[ship.index].second = min_harbor;
            return;
        }
        else{
            ResourceEnum best_resource = ResourceEnum::Gold;
            Harbor best_harbor = world.harbors[0];
            for(Harbor harbor : world.harbors){
                
            }
            
            // ship_orders[ship.index].first = 2;
            // ship_orders[ship.index].second = best_harbor.coords;
            return;
        
        }

             

    }
    else if(task.task == TaskEnum::GET_GOLD){

        if(ship.coords == world.my_base){
            turns.push_back(StoreTurn(ship.index, -min(ship.stats.max_cargo/2, world.gold)));
            return;
        }
        turns.push_back(MoveTurn(ship.index, move_to(ship, world.my_base, condition)));
        return;
    }

    else if(task.task == TaskEnum::BUY_RESOURCES){

        //asi su naoupak suranice !!
        if(world.mapa.tiles[ship.coords.y][ship.coords.x].type != TileEnum::TILE_HARBOR){

            //najde najblizsi harbor
            unordered_map<XY, std::pair<int, XY>> dist;
            vector<XY>& transitions = SMERY;
            vector<Harbor> harbors = world.harbors;
            bfs(ship.coords, condition, dist, transitions);
            int min_dist = 1000000;
            XY XY_harbor;

            
            for(int i=0;i<harbors.size();i++){
                bool found = false;
                for(pair<XY,int> pair : occupied_harbors){
                    if(pair.first == harbors[i].coords&&pair.second!=ship.index){
                        found = true;
                        break;
                    }
                }
                if(found){
                    continue;
                }
                if(dist[harbors[i].coords].first < min_dist){
                    min_dist = dist[harbors[i].coords].first;
                    XY_harbor = harbors[i].coords;
                }  
            }  
            

            //prejde na najblizsi harbor
            turns.push_back(MoveTurn(ship.index, move_to(ship, XY_harbor, condition)));
            add_to_occupant(XY_harbor, ship.index);
           
            task.task = TaskEnum::BUY_RESOURCES;
            
            task.coords = XY_harbor;
            return;



        }

        else{

            //som v harbore s peniazmi

            //najde najvyhodnejsi trade
            ResourceEnum best_resource = ResourceEnum::Gold;
            Harbor best_harbor = world.harbors[0];
            for(Harbor harbor : world.harbors){
                if(harbor.coords == ship.coords){
                    cerr<<"som v harbore"<<endl;
                    vector_harbor_instert(harbor);
                    cerr<<vector_of_found_harbors.size()<<endl;
                    if(vector_of_found_harbors.size()<2){
                        return;
                    }
                    best_resource = find_best_harbor(harbor).second;
                    best_harbor = find_best_harbor(harbor).first;
                    break;
                }
            }

            cerr<<"ship resources: "<<ship.resources[ResourceEnum::Gold]<<endl;
            cerr<<"best_harbor resource cost: "<<best_harbor.resource_cost(best_resource)<<endl;
            int resources_to_store = min(ship.resources[ResourceEnum::Gold]/best_harbor.resource_cost(best_resource), ship.stats.max_cargo);
            cerr<<resources_to_store<<endl;
            if(resources_to_store == 0){
                return;
            }
            turns.push_back(TradeTurn(ship.index, best_resource,resources_to_store));
            for(int i=0;i<occupied_harbors.size();i++){
                if(occupied_harbors[i].first == ship.coords){
                    occupied_harbors.erase(occupied_harbors.begin()+i);
                    break;
                }
            }

            
            task.task = TaskEnum::SELL_RESOURCES;
            
            task.coords = best_harbor.coords;
            return;
        
        }
    }
    else if(task.task == TaskEnum::SELL_RESOURCES){

        //predavanie materialu
        if(ship.coords != task.coords)
        turns.push_back(MoveTurn(ship.index,move_to(ship, task.coords, condition)));
        //cesta tam

        else{
            
            //predam vsetko co mam
            int resouces_to_sell ;
            for(int i=0;i<8;i++){
                if(ship.resources[ResourceEnum(i)] != 0){
                    resouces_to_sell=i;
                    cerr<<"predam: "<<resouces_to_sell<<endl;   
                    break;
                }
            }
            turns.push_back(TradeTurn(ship.index,ResourceEnum(resouces_to_sell),-ship.resources[ResourceEnum(resouces_to_sell)]));
            return;
        }
    }
}




pair<Ship, bool> enemy_ship_nearby(XY temp_coords, int range){
    for(Ship ship : world.ships){
        if(!ship.mine && abs(temp_coords.x - ship.coords.x) + abs(temp_coords.y - ship.coords.y)  <= range){
            return make_pair(ship, true);
        }
    }
    return make_pair(Ship(), false);
}

void add_attack_ship_turn(vector<Turn>& turns, pair<Ship, Task> ship_task){
    //TODO co ma robit attack ship

    Ship ship = ship_task.first;
    Task task = ship_task.second;

    pair<Ship, bool> enemy_ship = enemy_ship_nearby(ship.coords, ship.stats.range);

    if(enemy_ship.second){
        turns.push_back(ShootTurn(ship.index, enemy_ship.first.index));
        return;
    }

    unordered_map<XY, std::pair<int, XY>> dist;
    vector<Harbor> harbors = world.harbors;
    bfs(ship.coords, condition_for_attack_ship, dist); //TODO upravit condition

    int min_dist = 1000000;
    XY closest_ship;

    for(Ship curr: world.ships){
        if(curr.mine){
            
            if(dist[curr.coords].first < min_dist && dist[curr.coords].first != 0){
                min_dist = dist[curr.coords].first;
                closest_ship = curr.coords;
            }
        }
    }

    // cerr << world.my_ships() << endl;

    // cerr << "vybem sa na z " << ship.coords << "na " << move_to(ship, closest_ship, condition) << endl;
    // cerr << "najblizsi ship je na " << closest_ship << endl;
    turns.push_back(MoveTurn(ship.index, move_to(ship, closest_ship, condition)));
}

void add_loot_ship_turn(vector<Turn>& turns, Ship ship){
    //TODO co ma robit loot ship
}


void add_ship_turns(vector<Turn>& turns, vector<pair<Ship, Task>> ships){

    for (pair<Ship, Task> curr: ships)
    {
        if(curr.first.stats.ship_class == ShipClass::SHIP_TRADE){
            add_trade_ship_turn(turns, curr);
        }

        else if (curr.first.stats.ship_class == ShipClass::SHIP_ATTACK)
        {
            cerr << "som v attack ship" << endl;
            add_attack_ship_turn(turns, curr);
        }

        // else if (curr.stats.ship_class == ShipClass::SHIP_LOOT)
        // {
        //     add_loot_ship_turn(turns, curr);
        // }
    }

}



vector<Turn> do_turn() {
    tah++;
    vector<Turn> turns;
    for(int i=0;i<world.ships.size();i++){
        if(world.ships[i].mine){
            cerr<<world.ships[i].index<<" "<<world.ships[i].coords<<endl;
        }
    }
    for(int i=0;i<world.harbors.size();i++){
        cerr<<world.harbors[i].coords<<" ";
    }
    cerr<<endl;

    if(tah == 1){ // predpocitania na zaciatku hry
        for(Harbor harbor : world.harbors){
            coords_of_all_harbors.push_back(harbor.coords);
        }

        cerr << coords_of_all_harbors << endl;
        // for (int i = 0; i < 3; i++) //kupenie prvych 3 lodi
        // {
        //     ships_to_buy[i].second = true;
        //     turns.push_back(BuyTurn(ships_to_buy[i].first));
        // }
        
    }

    //TODO upavit tuto funkciu
   

    int gold_to_use = world.gold / 2;

    add_buy_turns(turns, gold_to_use);
    add_ship_turns(turns, our_ships);

    update_vector_of_ships(our_ships, our_ships_ids);


    cerr << "Takto mozete vypisovat do logov" << endl;
    cerr << turns << endl;
    return turns;
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
