#include "bullet.hpp"

#include <iostream>
#include <cstdlib>
#include <set>

struct Circle;

struct Player {
  struct Input {};

  Vec2 target;
  bool shoot = false;
  bool split = false;

  bool joined = false;

  std::vector<Circle*> circles;
  std::string name;
};

struct Circle : Item {
  double r;
  Vec2 pos, velocity;

  Player *player;
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;

  bool eaten = false;
  
  Circle(Vec2 pos, double r) : pos(pos), r(r) {
  }
  Aabb getAabb() const override {
    return {pos.x-r, pos.y-r, pos.x+r, pos.y+r};
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
  std::set<Circle*> circles;

  void addPellets(int count) {
    for (int i = 0; i < count; i++) {
      auto circle = new Circle({drand48()*1000, drand48()*1000}, 1);
      circle->type = Circle::Type::PELLET;
      circles.insert(circle);
      b.add(circle);
    }
  }

  void joinPlayer(Player *player) {
    if (player->joined) return;
    player->joined = true;

    auto circle = new Circle({drand48()*1000, drand48()*1000}, 50);
    circle->type = Circle::Type::PLAYER;
    circle->player = player;

    player->circles.push_back(circle);
    circles.insert(circle);
    players.push_back(player);
    b.add(circle);
  }

  void step() {
    //std::cout << "r =";
    for (auto p : players) {
      for (auto c : p->circles) {
        //std::cout << " " << c->r;
        c->velocity = (p->target - c->pos).normalize() * 0.5;
      }
      p->shoot = false; // TODO
      p->split = false; // TODO
    }
    //std::cout << "\n";
    for (auto c : circles) {
      if (c->velocity == Vec2 {0,0}) continue;
      c->pos += c->velocity;
      c->velocity = c->velocity.shorten(0.01);
    }
    b.update();
    b.calcPairs();
    for (auto p : b.pairs)
      woof(*static_cast<Circle*>(p.first), *static_cast<Circle*>(p.second));
    //for (auto fst : circles)
    //  for (auto snd : circles)
    //    woof(*fst, *snd);
    b.update();

    std::set<Circle*> eaten;
    for (auto c: circles) {
      if (c->eaten) {
        b.remove(c);
        if (c->player) {
          auto & vec = c->player->circles;
          vec.erase(std::remove(vec.begin(), vec.end(), c), vec.end());
        }
        delete c;
        eaten.insert(c);
      }
    }
    for (auto c: eaten)
      circles.erase(c);
  }

  void woof(Circle &fst, Circle &snd) {
    bool actionEat = false;
    bool actionCollide = false;
    bool actionExplode = false;
    
    double dist2 = (fst.pos - snd.pos).length2();
    bool canCollide = dist2 <= sqr(fst.r + snd.r);
    bool canEatD = dist2 <= sqr(fst.getEatingRange() + snd.r*0);
    bool canEatR = fst.r >= 1.25*snd.r || true;
    
    if (fst.type == Circle::Type::PLAYER) {
      if (snd.type == Circle::Type::PLAYER && fst.player == snd.player) {
        bool canMerge = false; // TODO
        if (!canMerge && canCollide)
          actionCollide = true;
        else if (canMerge && canEatD)
          actionEat = true;
      } else if (snd.type == Circle::Type::PELLET && canCollide) {
        std::cout << "E\n";
        actionEat = true;
      } else if (snd.type == Circle::Type::PLAYER && canEatD && canEatR) { // ???
        //std::cout << "Eat: " << dist2 << " " <<  sqr(fst.getEatingRange() + snd.r) << "\n";
        //std::cout << "     " << fst.r << " " << fst.getEatingRange() << " " << snd.r << "\n";
        actionEat = true;
        if (snd.type == Circle::Type::VIRUS)
          actionExplode = true;
      }
    } else if (fst.type == Circle::Type::VIRUS &&
               snd.type == Circle::Type::FOOD &&
               canEatD) {
      actionEat = true;
      // TODO: split virus
    }

    if (actionEat && !fst.eaten && !snd.eaten) {
      fst.r += snd.r;
      snd.eaten = true;
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
  //players[0].circles[0]->pos = {0,0};
  //players[1].circles[0]->pos = {100,0};
  players[0].target = {100, 0};

  game.addPellets(2000);
  char fname[32];
  
  for (int i = 0; i < 1000; i++) {
    sprintf(fname, "out/%04d.svg", i);
    if(i % 1 == 0) game.svg(fname);
    
    game.step();
  }
}
