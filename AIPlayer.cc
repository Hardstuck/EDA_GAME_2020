#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Niebo

// DISCLAIMER: The following Demo playercitizen is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.

struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }


  /**
   * Types and attributes for your player can be defined here.
   */
  const vector<Dir> dirs = {Up,Down,Left,Right};
  const vector<BonusType> options = {Food, Money};

/*  void search_bonus(map<Pos, int>& command, id, vector<Pos> occupied){
    Pos tmpt = builder_day(id);
    auto it = command.find(tmpt);
    if(it != command.end()){
      if(manhattan_distance(tmpt, *it) > manhattan_distance(tmpt, citizen(id).pos)){
        search_bonus(command, id);
        command[tmpt] = id;
      }
    }
  }*/

  void move_builder(){
    Pos aux;
    vector<int> b = builders(me());
    vector<Pos> occupied;
    for(int id : b){
      Pos tmpt;
      if (is_day()) {
        if (!builder_day(id, occupied, aux)) {
          move(id, direction(aux, citizen(id).pos));
          occupied.push_back(tmpt);
        }
        else build(id, direction(aux, citizen(id).pos));
      }
      else {
        tmpt = builder_night(id, occupied);
        if(tmpt != citizen(id).pos) move(id, direction(tmpt, citizen(id).pos));
        occupied.push_back(tmpt);
      }
    }
  }

  void move_warrior(){
    Pos aux;
    vector<int> b = warriors(me());
    vector<Pos> occupied;
    for(int id : b){
      Pos tmpt;
      if (is_day()) {
        tmpt = warrior_day(id, occupied);
        occupied.push_back(tmpt);
        move(id, direction(tmpt, citizen(id).pos));
      }
      else {
        tmpt = warrior_night(id, occupied);
        move(id, direction(tmpt, citizen(id).pos));
      }
    }
  }

  void print_stack(queue<Pos> k){
    queue<Pos> aux = k;
    cerr << "stack content:" << endl;
    while(not aux.empty()){
      Pos i = aux.front();
      aux.pop();
      cerr << i.i << " " << i.j << endl;
    }
    return;
  }
  
  Dir direction(Pos next, Pos actual){
    for(Dir d : dirs) if(actual + d == next) return d;
    cerr << "something is wrong" << endl;
    return dirs[0];
  }
  
  Dir counter_direction(Pos enemy, Pos me){
    int distance = manhattan_distance(enemy, me);
    for(Dir d : dirs) if(pos_ok(me + d) and manhattan_distance(me + d, enemy) > distance) return d;
    return dirs[0];
  }
  
  int manhattan_distance(Pos x, Pos y){
    return abs(x.i - y.i) + abs(x.j - y.j);
  }
  
  Pos first(int id, Pos O, const vector<vector<Pos>> &map){
    Pos ini = citizen(id).pos;
    Pos previous = O;
    while(map[previous.i][previous.j] != ini) previous = map[previous.i][previous.j];
    return previous;
  }
  
  bool bfs_barricade(int id, Pos& pos){
    if( cell(citizen(id).pos).b_owner == me()) {
      pos = citizen(id).pos;
      return false;
    }
    Pos ini = citizen(id).pos;
    Pos next;
    //TODO change parameters
    vector<vector<Pos>> tracking(15, vector<Pos>(30));
    vector<vector<bool>> visited(15, vector<bool>(30, false));
    queue<Pos> Q;
    Q.push(ini);
    visited[ini.i][ini.j] = true;
    while(!Q.empty()){
      ini = Q.front();
      Q.pop();
      for(Dir d : dirs){
        next = ini + d;
        if(pos_ok(next) && cell(next).type == Street && !visited[next.i][next.j] && cell(next).id == -1){
          visited[next.i][next.j] = true;
          if(cell(next).b_owner == me()){
            tracking[next.i][next.j] = ini;
            pos = first(id, next, tracking);
            if(manhattan_distance(pos, citizen(id).pos) == 1) return true;
            return false;
          }
          else if(cell(next).b_owner == -1) {
            tracking[next.i][next.j] = ini;
            Q.push(next);
          }
        }
      }
    }
    for (Dir d : dirs) if(pos_ok(citizen(id).pos + d) and cell(citizen(id).pos + d).id == -1){
      pos = citizen(id).pos + d;
      return true;
    }
    pos = citizen(id).pos;
    return false;
  }

  Pos bfs_enemy(int id){
    Pos ini = citizen(id).pos;
    Pos next;
    // TODO change parameters
    vector<vector<bool>> visited(15, vector<bool>(30, false));
    queue<Pos> Q;
    Q.push(ini);
    visited[ini.i][ini.j] = true;
    while(!Q.empty()){
      ini = Q.front();
      Q.pop();
      for(Dir d : dirs){
        next = ini + d;
        if(pos_ok(next) and cell(next).type == Street && cell(next).b_owner == -1 and !visited[next.i][next.j]){
          visited[next.i][next.j] = true;
          if (cell(next).id != -1 && citizen(cell(next).id).player != me()) return next;
          else if(cell(next).id == -1) Q.push(next);
        }
      }
    }
    // TODO not enemy found
    return ini;
  }

  Pos bfs_kill(int id, vector<Pos> occupied){
    WeaponType weapon = citizen(id).weapon;
    Pos ini = citizen(id).pos;
    Pos next;
    // TODO change parameters
    vector<vector<Pos>> tracking(board_rows(), vector<Pos>(board_cols()));
    vector<vector<bool>> visited(15, vector<bool>(30, false));
    queue<Pos> Q;
    Q.push(ini);
    visited[ini.i][ini.j] = true;
    while(!Q.empty()){
      ini = Q.front();
      Q.pop();
      for(Dir d : dirs){
        next = ini + d;
        if(pos_ok(next) and cell(next).type == Street && cell(next).b_owner == -1 and !visited[next.i][next.j]){
          if(cell(next).b_owner == -1 or cell(next).b_owner != me()){
            visited[next.i][next.j] = true;
            tracking[next.i][next.j] = ini;
            if (cell(next).id != -1 && citizen(cell(next).id).player != me()) {
              if(weapon == Bazooka and citizen(cell(next).id).weapon != Bazooka) return first(id, next, tracking);
              else if(weapon == Gun and (citizen(cell(next).id).weapon == Hammer or citizen(cell(next).id).weapon == NoWeapon)) return first(id, next, tracking);
              else if(weapon == Hammer and citizen(cell(next).id).weapon == NoWeapon) return first(id, next, tracking);
            }
            else if(cell(next).id == -1) Q.push(next);
          }
          else if(cell(next).b_owner != -1 and citizen(cell(next).id).player != me()){
            visited[next.i][next.j] = true;
            tracking[next.i][next.j] = ini;
            if (cell(next).id != -1 && citizen(cell(next).id).player != me()) {
              if(citizen(cell(next).id).type == Builder) return first(id, next, tracking);
            }
            else if(cell(next).id == -1) Q.push(next);
          }
        }
      }
    }
    // TODO not enemy found
    return ini;
  }

  Pos bfs(int id, BonusType option, vector<Pos> occupied){
    vector<vector<Pos>> tracking(board_rows(), vector<Pos>(board_cols()));
    vector<vector<bool>> visited(board_rows(), vector<bool>(board_cols(), false));
    Pos ini = citizen(id).pos;
    Pos next;
    queue<Pos> Q;
    Q.push(ini);
    visited[ini.i][ini.j] = true;
    for (Pos o : occupied) visited[o.i][o.j] = true;
    while(!Q.empty()){
      ini = Q.front();
      Q.pop();
      for(Dir d : dirs){
        next = ini + d;
        if(pos_ok(next) && cell(next).type == Street && !visited[next.i][next.j] && cell(next).id == -1 && cell(next).b_owner == -1){
          visited[next.i][next.j] = true;
          tracking[next.i][next.j] = ini;
          if (cell(next).bonus == option) {
            return first(id, next, tracking);
          }
          else Q.push(next);
        }
      }
    }
    //TODO if not found any bonus what shall I do?
    return ini;
  }
  
  Pos bfs_weapon(int id, WeaponType weapon, const vector<Pos> occupied){
    Pos ini = citizen(id).pos;
    Pos next;
    //TODO change parameters
    vector<vector<Pos>> tracking(15, vector<Pos>(30));
    vector<vector<bool>> visited(15, vector<bool>(30, false));
    queue<Pos> Q;
    Q.push(ini);
    for(Pos o : occupied) visited[o.i][o.j] = true;
    visited[ini.i][ini.j] = true;
    while(!Q.empty()){
      ini = Q.front();
      Q.pop();
      for(Dir d : dirs){
        next = ini + d;
        if(pos_ok(next) and cell(next).type == Street and (cell(next).b_owner == me() or cell(next).b_owner == -1) and cell(next).id == -1 and !visited[next.i][next.j]){
          visited[next.i][next.j] = true;
          tracking[next.i][next.j] = ini;
          if(cell(next).weapon != NoWeapon){
            if(cell(next).weapon == Bazooka) return first(id, next, tracking);
            else if(cell(next).weapon == Gun and (weapon == Gun or weapon == Hammer or weapon == NoWeapon)) return first(id, next, tracking);
            else if(cell(next).weapon == Hammer and (weapon == Hammer or weapon == NoWeapon)) return first(id, next, tracking);
            else Q.push(next);
          }
          else Q.push(next);
        }
      }
    }
    return ini;
  }

  bool builder_day(int id, vector<Pos> occupied, Pos& pos){
    Pos ini = citizen(id).pos;
    if(round()%25 < 18) {
      Pos food = bfs(id, options[0], occupied);
      Pos money = bfs(id, options[1], occupied);
      if (citizen(id).life == citizen_ini_life(Builder)) pos = money;
      else {
        int d1, d2;
        d1 = manhattan_distance(ini, food);
        d2 = manhattan_distance(ini, money);
        if (d2 + 3 < d1) pos = money;
        else pos = food;
      }
      return false;
    }
    else {
      bool aux = bfs_barricade(id, pos);
      if (cell(pos).resistance == barricade_max_resistance()) {
        pos = bfs(id, options[1], occupied);
        return false;
      }
      else return aux;
    }
  }
  
  Pos builder_night(int id, const vector<Pos> occupied){
    //enemy close run to barricades otherwise go for money not too far
    Pos O = citizen(id).pos;
    for (Dir d : dirs) if(pos_ok(O + d) and cell(O + d).id != -1 and citizen(cell(O + d).id).player != me() and citizen(cell(O + d).id).type == Builder) return O + d;
    Pos closer_barricade;
    Pos closer_enemy = bfs_enemy(id);
    bfs_barricade(id, closer_barricade);
    int distance_enemy_me = manhattan_distance(closer_enemy, O);
    if(distance_enemy_me < 3) {
      if(distance_enemy_me == 1){
        for(Dir d : dirs){
          Pos aux = citizen(id).pos + d;
          if((pos_ok(aux) && cell(aux).type == Street && cell(aux).b_owner == -1) && cell(aux).id == -1) {
            for(Dir d2 : dirs){
              Pos aux2 = aux + d2;
              if(pos_ok(aux2) && (cell(aux2).type == Street) && (cell(aux2).id == -1 || citizen(cell(aux2).id).player == me())) return aux;
            }
          }
        }
      }
      return closer_barricade;
    }
    else {
      Pos money = bfs(id, options[1], occupied);
      if(citizen(id).life < citizen_ini_life(Builder) - 20) {
        Pos food = bfs(id, options[0], occupied);
        if (manhattan_distance(food, closer_enemy) > manhattan_distance(food, O)) return food;
        else if (manhattan_distance(money, closer_enemy) > manhattan_distance(money, O)) return money;
        else move(id, counter_direction(closer_enemy, O));
      }
      else{
        if (manhattan_distance(money, closer_enemy) > manhattan_distance(money, O)) return money;
        else return citizen(id).pos + counter_direction(closer_enemy, O);
      }
    }
    //TODO me?
    return O;
  }

  Pos warrior_day(int id, vector<Pos> occupied){
    Pos pos;
    if(round() % 25 > 22){
      return bfs_kill(id, occupied);
    }
      if(citizen(id).life < citizen_ini_life(Warrior) * 2/3) pos = bfs(id, options[0], occupied);
      else if(citizen(id).weapon == Bazooka) {
        if(citizen(id).life < citizen_ini_life(Warrior)) pos = bfs(id, options[0], occupied);
        else pos = bfs(id, options[1], occupied);
      }
      else {
        pos = bfs_weapon(id, citizen(id).weapon, occupied);
        if (pos == citizen(id).pos) pos = bfs(id, options[1], occupied);
      }
      return pos;
  }

  Pos warrior_night(int id, vector<Pos>& occupied){
    Pos ini = citizen(id).pos;
    /*for(Dir d : dirs) {
      if(pos_ok(ini + d) && cell(ini+d).type == Street && (cell(ini+d).id != me() && cell(ini+d).id != -1)){
        if (citizen(id).weapon == Bazooka) return ini + d;
        else if(citizen(id).weapon == Gun && (citizen(cell(ini+d).id).weapon == Gun or citizen(cell(ini+d).id).weapon == Hammer or citizen(cell(ini+d).id).weapon == NoWeapon)) return ini + d;
        else if(citizen(id).weapon == Hammer && (citizen(cell(ini+d).id).weapon == Hammer or citizen(cell(ini+d).id).weapon == NoWeapon)) return ini + d;
      }
    }*/
    Pos tmpt = bfs_kill(id, occupied);
    if(tmpt != citizen(id).pos) occupied.push_back(tmpt);
    if(tmpt == citizen(id).pos) {
        tmpt = bfs_weapon(id, citizen(id).weapon, occupied);
        occupied.push_back(tmpt);
    }
    if(tmpt == citizen(id).pos) {
        tmpt = bfs(id, options[1], occupied);
        occupied.push_back(tmpt);
    }
    return tmpt;
  }
  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    move_builder();
    move_warrior();
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
