#include "bullet.hpp"
#include "svg.hpp"
#include "ws.hpp"
#include "bytes.hpp"

#include <iostream>
#include <cstdlib>
#include <mutex>
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
  unsigned id;
  Vec2 target;
  bool shoot = false;
  bool split = false;

  enum class Mode {
    DEFAULT, SPECTRATE, GAME
  } mode;

  std::set<PlayerCell*> cells;
  std::u16string name;
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

struct Game;

namespace update {

struct Update {
  virtual void woof(Player *, Game *) {}
};

struct Spawn : Update {
  std::u16string name;
  Spawn(BytesOut &b)
  { name = b.getU16String(); }
};

struct Join : Update {
};

struct Direction : Update {
  double x, y;
  uint32_t width;
  Direction(BytesOut &b)
  { x = b.get<double>();
    y = b.get<double>();
    width = b.get<uint32_t>(); }
};

struct Split : Update {
};

struct Eject : Update {
};

struct AFK : Update {
};

struct Explode : Update {
};

struct Q : Update {
};

struct Token : Update {
  std::string token;
  Token(BytesOut &b)
  { b.pos = b.len; /* just ignore rest bytes */ }
};

struct Error : Update {
};

Update *parse0(BytesOut &b) {
  switch (b.getByte()) {
    case   0: return new Spawn(b);
    case   1: return new Join;
    case  16: return new Direction(b);
    case  17: return new Split;
    case  18: return new Eject;
    case  19: return new AFK;
    case  20: return new Explode;
    case  21: return new Q;
    case  80: return new Token(b);
    case 254:
    case 255: return new Q(); // :3
  }
  return 0;
}

Update *parse(BytesOut &b) {
  Update *update = parse0(b);
  if (!update || b.len != b.pos) {
    delete update;
    return new Error;
  }
  return update;
}

}

struct Game {
  Broadphase b;
  struct GameWsServer : WsServer {
    Game &game;
    GameWsServer(Game &game) : game(game), WsServer(8000) {}
    Player *wsOnConnect() override {
      Player *p = new Player;
      return p;
      //game.joinPlayer(p);
    }
    void wsOnReceive(Player *p, const char *data, size_t len) override {
      BytesOut b {data, len, 0};
      auto type = b.get<uint8_t>();
      switch (type) {
        case   0: //nickname
          
          //std::u16string();
          break;
        case 254: // hello
        case 255: // hello
        case  80: // Connection Token
          break;
      }
    }
    void wsOnDisconnect(Player *p) override {
    }
  } ws;

  Game() : ws(*this) {}

  struct Updates {
    std::mutex mutex;
    std::set<Player *> joinedPlayers;
    std::set<Player *> leavedPlayers;
  } updates;

  std::set<Player *> players;
  std::set<Cell *> cells;

  void addPellets(int count) {
    for (int i = 0; i < count; i++) {
      auto cell = new FoodCell({drand48()*1000, drand48()*1000}, 1);
      cells.insert(cell);
      b.add(cell);
    }
  }

  void joinPlayer(Player *player) {
    if (player->mode == Player::Mode::GAME) return;
    player->mode == Player::Mode::GAME;

    auto cell = new PlayerCell(player, {drand48()*1000, drand48()*1000}, 10);
    cells.insert(cell);
    b.add(cell);
    players.insert(player);
  }

  void handleUpdates() {
    std::lock_guard<std::mutex> lock(updates.mutex);
    
    
    updates.joinedPlayers.clear();
    updates.leavedPlayers.clear();
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
  
  /*
  for (int i = 0; i < 1000 || 1; i++) {
    sprintf(fname, "out/%04d.svg", i);
    //if(i % 10 == 0) game.svg(fname);
    game.step();
  }
  */
  game.ws.wsRun();
  while(1);
  //game.stop();
}
