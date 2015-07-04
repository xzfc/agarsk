#include "bullet.hpp"
#include "svg.hpp"

#include <iostream>
#include <cstdlib>
#include <set>

struct Cell;

struct Player {
  Vec2 target;
  bool shoot = false;
  bool split = false;

  bool joined = false;

  std::set<Cell*> cells;
  std::string name;
};

struct Cell : Item {
  double r;
  Vec2 pos, velocity;

  Player *player;
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;

  bool eaten = false;
  bool moved = false;
  
  Cell(Vec2 pos, double r) : pos(pos), r(r) {
  }
  virtual ~Cell() {}
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

struct PlayerCell : Cell {
  Player *player;

  PlayerCell(Player *p, Vec2 pos, double r) : Cell(pos, r) {
    player = p;
    type = Type::PLAYER;
    player->cells.insert(this);
  }
  virtual ~PlayerCell() override {
    player->cells.erase(this);
    // todo:check if player have other cells
  }
};

struct Food : Cell {
  Food(Vec2 pos, double r) : Cell(pos, r) {
    type = Type::FOOD;
  }
};

struct Game {
  Broadphase b;

  std::vector<Player*> players;
  std::set<Cell*> cells;
  std::vector<std::pair<Cell*, Cell*>> eaten;

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

    player->cells.insert(cell);
    cells.insert(cell);
    players.push_back(player);
    b.add(cell);
  }

  void removeCell(Cell *c) {
    b.remove(c);
    cells.erase(c);
    delete c;
  }

  void step() {
    for (auto p : players) {
      for (auto c : p->cells)
        c->velocity = (p->target - c->pos).normalize() * 0.5;
      p->shoot = false; // TODO
      p->split = false; // TODO
    }
    
    for (auto c : cells)
      if (c->velocity == Vec2 {0,0}) {
        c->moved = false;
      } else {
        c->pos += c->velocity;
        c->velocity = c->velocity.shorten(0.01);
        c->moved = true;
      }

    eaten.clear();
    
    for (auto p : b.getCollisions()) {
      Cell *fstCell = dynamic_cast<Cell*>(p.first),
           *sndCell = dynamic_cast<Cell*>(p.second);
      if (fstCell && sndCell) {
        if (fstCell->r < sndCell->r)
          std::swap(fstCell, sndCell);
        handleInteraction(*fstCell, *sndCell);
      }
    }

    for (auto p : eaten) {
      Cell *c = p.second;
      b.remove(c);
      if (c->type == Cell::Type::PLAYER) {
        c->player->cells.erase(c);
        // todo: check if player have cells
      }
      cells.erase(c);
      delete c;
    }

    // new state in: eaten; cells.filter(c-> c.moved)
  }

  void handleInteraction(Cell &fst, Cell &snd) {    
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
      snd.eaten = true;
      eaten.push_back({&fst, &snd});
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
