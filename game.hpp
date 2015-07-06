#pragma once
#include "bullet.hpp"
#include "player.hpp"

#include <mutex>

struct Cell;
struct PlayerCell;

struct Modifications {
  std::vector<std::pair<Cell*, Cell*>> eaten;
  std::vector<Cell *> removed;
  std::vector<Cell *> added;
  std::vector<Cell *> moved;
};

struct Cell : Item {
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;
  unsigned mass_;
  double r;
  Vec2 pos, velocity;
  
  bool eaten = false;
  bool moved = false;
  
  Cell(Vec2 pos, unsigned mass);

  unsigned mass();

  void mass(int mass);
  
  virtual ~Cell() {}
  
  Aabb getAabb() const override;
  
  Aabb getPotentialAabb() const override;
  
  virtual void svg(struct Svg &s) const;
  virtual void step(Modifications &m);
};

struct PlayerCell : Cell {
  Player *player;
  bool exploded = false;

  PlayerCell(Player *p, Vec2 pos, unsigned mass);
  ~PlayerCell() override;
  PlayerCell *split(Modifications &m, double size);
  void step(Modifications &m) override;
};

struct FoodCell : Cell {
  FoodCell(Vec2 pos, unsigned mass);
};

struct Game {
  Broadphase b;
  std::set<Player *> players;
  std::set<Cell *> cells;

  void addPellets(int count);
  void joinPlayer(Player *player);
  void step();
  void handleInteraction(Modifications &m, Cell *fst, Cell *snd);
  void stop();
  void svg(const char *fname);
};
