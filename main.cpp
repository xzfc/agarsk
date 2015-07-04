#include "bullet.hpp"
#include "svg.hpp"

#include <iostream>
#include <cstdlib>
#include <set>

struct Cell;

struct Player {
  struct Input {};

  Vec2 target;
  bool shoot = false;
  bool split = false;

  bool joined = false;

  std::vector<Cell*> cells;
  std::string name;
};

struct Cell : Item {
  double r;
  Vec2 pos, velocity;

  Player *player;
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;

  bool eaten = false;
  
  Cell(Vec2 pos, double r) : pos(pos), r(r) {
  }
  Aabb getAabb() const override {
    return {pos.x-r, pos.y-r, pos.x+r, pos.y+r};
  }
  Aabb getPotentialAabb() const override {
    if (type == Type::PELLET)
      return getAabb();
    else
      return getAabb().expand(3);
  }
  void svg(Svg &s) const override {
    s.circle(pos.x, pos.y, r, "none", "rgba(255,0,  0,  0.3)");
  }
  double getEatingRange() const {
    if (type == Type::PLAYER || type == Type::VIRUS)
      return r * 0.4;
    return 0.;
  }
};

struct Game {
  Broadphase b;

  std::vector<Player*> players;
  std::set<Cell*> cells;

  void addPellets(int count) {
    for (int i = 0; i < count; i++) {
      auto cell = new Cell({drand48()*1000, drand48()*1000}, 1);
      cell->type = Cell::Type::PELLET;
      cells.insert(cell);
      b.add(cell);
    }
  }

  void joinPlayer(Player *player) {
    if (player->joined) return;
    player->joined = true;

    auto cell = new Cell({drand48()*1000, drand48()*1000}, 50);
    cell->type = Cell::Type::PLAYER;
    cell->player = player;

    player->cells.push_back(cell);
    cells.insert(cell);
    players.push_back(player);
    b.add(cell);
  }

  void step() {
    //std::cout << "r =";
    for (auto p : players) {
      for (auto c : p->cells) {
        //std::cout << " " << c->r;
        c->velocity = (p->target - c->pos).normalize() * 0.5;
      }
      p->shoot = false; // TODO
      p->split = false; // TODO
    }
    //std::cout << "\n";
    for (auto c : cells) {
      if (c->velocity == Vec2 {0,0}) continue;
      c->pos += c->velocity;
      c->velocity = c->velocity.shorten(0.01);
    }
    for (auto p : b.getCollisions()) {
      auto &fst = *static_cast<Cell*>(p.first),
           &snd = *static_cast<Cell*>(p.second);
      if (fst.r > snd.r)
        woof(fst, snd);
      else
        woof(snd, fst);
    }

    std::set<Cell*> eaten;
    for (auto c: cells) {
      if (c->eaten) {
        b.remove(c);
        if (c->type == Cell::Type::PLAYER) {
          auto & vec = c->player->cells;
          vec.erase(std::remove(vec.begin(), vec.end(), c), vec.end());
        }
        delete c;
        eaten.insert(c);
      }
    }
    for (auto c: eaten)
      cells.erase(c);
  }

  void woof(Cell &fst, Cell &snd) {
    bool actionEat = true;
    bool actionCollide = false;
    bool actionExplode = false;
    
    double dist2 = (fst.pos - snd.pos).length2();
    bool canCollide = dist2 <= sqr(fst.r + snd.r);
    bool canEatD = dist2 <= sqr(fst.getEatingRange() + snd.r*0);
    bool canEatR = fst.r >= 1.25*snd.r || true;
    
    if (fst.type == Cell::Type::PLAYER) {
      if (snd.type == Cell::Type::PLAYER && fst.player == snd.player) {
        bool canMerge = false; // TODO
        if (!canMerge && canCollide)
          actionCollide = true;
        else if (canMerge && canEatD)
          actionEat = true;
      } else if (snd.type == Cell::Type::PELLET && canCollide) {
        actionEat = true;
      } else if (snd.type == Cell::Type::PLAYER && canEatD && canEatR) { // ???
        //std::cout << "Eat: " << dist2 << " " <<  sqr(fst.getEatingRange() + snd.r) << "\n";
        //std::cout << "     " << fst.r << " " << fst.getEatingRange() << " " << snd.r << "\n";
        actionEat = true;
        if (snd.type == Cell::Type::VIRUS)
          actionExplode = true;
      }
    } else if (fst.type == Cell::Type::VIRUS &&
               snd.type == Cell::Type::FOOD &&
               canEatD) {
      actionEat = true;
      // TODO: split virus
    }

    if (actionEat && !fst.eaten && !snd.eaten) {
      //fst.r += snd.r;
      //snd.eaten = true;
    }

    if (actionCollide) {
      //normal(fst.pos-snd.pos);
      // TODO
    }

    if (actionExplode && !fst.eaten) {
      // TODO
    }
  }

  void svg(const char *fname) {
    b.svg(fname);
  }
};

int main() {
  Game game;
  
  Player players[3];
  for (auto &p : players)
    game.joinPlayer(&p);
  //players[0].cells[0]->pos = {0,0};
  //players[1].cells[0]->pos = {100,0};
  players[0].target = {100, 0};

  game.addPellets(20000);
  char fname[32];
  
  for (int i = 0; i < 100; i++) {
    sprintf(fname, "out/%04d.svg", i);
    if(i % 100 == 0) game.svg(fname);
    
    game.step();
  }
}
