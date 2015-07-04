#include "bullet.hpp"
#include "svg.hpp"

#include <iostream>
#include <cstdlib>
#include <set>

struct Cell;
struct PlayerCell;

struct Modifications {
  std::vector<std::pair<Cell*, Cell*>> eaten;
  std::vector<Cell *> removed;
  std::vector<Cell *> added;
  std::vector<Cell *> moved;
};

struct Player {
  Vec2 target;
  bool shoot = false;
  bool split = false;

  bool joined = false;

  std::set<PlayerCell*> cells;
  std::string name;
  uint64_t color = 0;
};

struct Cell : Item {
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;
  unsigned mass_;
  double r;
  Vec2 pos, velocity;
  
  bool eaten = false;
  bool moved = false;
  
  Cell(Vec2 pos, unsigned mass) : pos(pos)
  { this->mass(mass); }

  unsigned mass()
  { return mass_; }

  void mass(int mass)
  { this->mass_ = mass;
    r = 10. * sqrt((double)mass_); }
  
  virtual ~Cell() {}
  
  Aabb getAabb() const override
  { return {pos.x-r, pos.y-r, pos.x+r, pos.y+r}; }
  
  Aabb getPotentialAabb() const override {
    return type == Type::PELLET ? getAabb() : getAabb().expand(3);
  }
  
  virtual void svg(Svg &s) const
  { s.circle(pos.x, pos.y, r, "none", "rgba(255,0,  0,  0.3)"); }

  virtual void step(Modifications &m) {
    if (velocity != Vec2 {0,0}) {
      pos += velocity;
      velocity = velocity.shorten(0.01);
      m.moved.push_back(this);
    }
  }
};

struct PlayerCell : Cell {
  Player *player;
  bool exploded = false;

  PlayerCell(Player *p, Vec2 pos, unsigned mass) : Cell(pos, mass) {
    player = p;
    type = Type::PLAYER;
    player->cells.insert(this);
  }
  ~PlayerCell() override {
    player->cells.erase(this);
    // todo:check if player have other cells
  }
  PlayerCell *split(Modifications &m, double size) {
    if (player->cells.size() >= 16)
      return nullptr;
    auto newCell = new PlayerCell(player, pos, mass() * size);
    newCell->pos.x += 0.1*(drand48()-0.5);
    newCell->pos.y += 0.1*(drand48()-0.5);
    mass(mass() * (1-size));
    m.added.push_back(newCell);
    return newCell;
  }
  void step(Modifications &m) override {
    Cell::step(m);
    if (exploded) {
      PlayerCell *c = this;
      do {
        c = c->split(m, 0.5);
      } while(c);
      exploded = false;
    }
  }
};

struct FoodCell : Cell {
  FoodCell(Vec2 pos, unsigned mass) : Cell(pos, mass) {
    type = Type::FOOD;
  }
};

struct Game {
  Broadphase b;

  std::vector<Player*> players;
  std::set<Cell*> cells;

  void addPellets(int count) {
    for (int i = 0; i < count; i++) {
      auto cell = new FoodCell({drand48()*1000, drand48()*1000}, 1);
      cells.insert(cell);
      b.add(cell);
    }
  }

  void joinPlayer(Player *player) {
    if (player->joined) return;
    player->joined = true;

    auto cell = new PlayerCell(player, {drand48()*1000, drand48()*1000}, 10);
    cells.insert(cell);
    b.add(cell);
    players.push_back(player);
  }

  void step() {
    Modifications m;
    for (auto c : cells)
      c->step(m);

    for (auto p : players) {
      for (auto c : p->cells)
        c->velocity = (p->target - c->pos).normalize() * 0.5;
      p->shoot = false; // TODO
      p->split = false; // TODO
    }
    
    for (auto p : b.getCollisions()) {
      Cell *fstCell = dynamic_cast<Cell*>(p.first),
           *sndCell = dynamic_cast<Cell*>(p.second);
      if (fstCell && sndCell) {
        if (fstCell->r < sndCell->r)
          std::swap(fstCell, sndCell);
        handleInteraction(m, fstCell, sndCell);
      }
    }

    for (auto p : m.eaten) {
      p.first->mass(p.first->mass() + p.second->mass());
      b.remove(p.second);
      cells.erase(p.second);
      delete p.second;
    }

    for (auto c : m.added) {
      cells.insert(c);
      b.add(c);
    }
  }

  void handleInteraction(Modifications &m, Cell *fst, Cell *snd) {    
    bool actionEat = false;
    bool actionCollide = false;
    bool actionExplode = false;
    
    double dist = (fst->pos - snd->pos).length();
    bool canCollide = dist <= fst->r + snd->r;
    bool canEatD = fst->r - dist - 0.354*snd->r >= -11;
    bool canEatR = 4*fst->mass() >= 5*snd->mass();

    auto fstP = dynamic_cast<PlayerCell *>(fst);
    auto sndP = dynamic_cast<PlayerCell *>(snd);
    
    if (fstP) {
      if (sndP && fstP->player == sndP->player) {
        bool canMerge = false; // TODO
        if (!canMerge && canCollide)
          actionCollide = true;
        else if (canMerge && canEatD)
          actionEat = true;
      } else if (snd->type == Cell::Type::FOOD && canEatD) {
        actionEat = true;
        //if (fst->mass() > 20)
        //  actionExplode = true;
      } else if (canEatD && canEatR) {
        actionEat = true;
        if (snd->type == Cell::Type::VIRUS)
          actionExplode = true;
      }
    } else if (fst->type == Cell::Type::VIRUS &&
               snd->type == Cell::Type::FOOD &&
               canEatD) {
      actionEat = true;
      // TODO: split virus
    }

    if (actionEat && !fst->eaten && !snd->eaten) {
      snd->eaten = true;
      m.eaten.push_back({fst, snd});
    }

    if (actionCollide) {
      auto diff = (fst->pos-snd->pos);
      auto vec = diff.normalize() * (-diff.length()+fst->r+snd->r) * 0.1;
      fst->velocity += vec;
      snd->velocity -= vec;
    }

    if (fstP && actionExplode && !fst->eaten)
      fstP->exploded = true;
  }

  void stop() {
    for (auto c: cells) {
      b.remove(c);
      delete c;
    }
    cells.clear();
    /*
    for (auto p: players)
      delete p;
    */
    players.clear();
  }

  void svg(const char *fname) {
    Svg svg(fname, {-100,-100,1000,1000}, 512, 512);
    for (Cell *c : cells)
      c->svg(svg);
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

  game.addPellets(2000);
  char fname[32];
  
  for (int i = 0; i < 1000; i++) {
    sprintf(fname, "out/%04d.svg", i);
    //if(i % 10 == 0) game.svg(fname);
    game.step();
  }
  game.stop();
}
