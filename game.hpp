#pragma once
#include <mutex>

#include "bullet.hpp"
#include "player.hpp"


struct Cell;
struct PlayerCell;

struct Modifications {
  std::vector<std::pair<Cell *, Cell *>> eaten;
  std::vector<Cell *> removed;
  std::vector<Cell *> added;
  std::vector<Cell *> moved;
  void clear();
};

struct Cell : Item {
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;
  unsigned mass_;
  double r;
  Vec2 pos, velocity;
  bool eaten = false;
  bool moved = false;
  uint32_t color = 0xFFFFFF;
  int id = 1;
  std::u16string name;
  
  Cell(Vec2 pos, unsigned mass);
  virtual ~Cell() {}
  unsigned mass();
  void mass(int mass);
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
  Aabb size {0,-0,1000,1000};
  std::set<Player *> players;
  std::set<Cell *> cells;
  Modifications mod;
  
  void addPellets(int count);
  void joinPlayer(Player *player);
  void step();
  void handleInteraction(Cell *fst, Cell *snd);
  void stop();
  void svg(const char *fname);
};
