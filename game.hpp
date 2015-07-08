#pragma once
#include <mutex>

#include "bullet.hpp"
#include "player.hpp"


struct Cell;
struct PlayerCell;

typedef uint32_t CellId;

struct Modifications {
  std::vector<std::pair<CellId, CellId>> eaten;
  std::vector<Cell *> deleted;
  std::vector<Cell *> added;
  void clear();
};

struct Cell : Item {
  // Static parameters
  enum class Type { PLAYER, VIRUS, PELLET, FOOD } type;
  uint32_t color = 0xFFFFFF;
  CellId id = 1;
  std::u16string name;

  // Dynamic parameters (mass, radius)
  unsigned mass();
  void mass(int mass);
  unsigned mass_;
  double r;
  // Dynamic parameters (other)
  Vec2 pos, velocity;

  // State changes flags
  bool updated = false;
  unsigned newMass;
  bool eaten = false;
  
  Cell(Vec2 pos, unsigned mass);
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
