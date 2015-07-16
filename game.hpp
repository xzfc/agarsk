#pragma once
#include <random>

#include "bullet.hpp"
#include "player.hpp"

struct Game;
struct Cell;
struct PlayerCell;

typedef uint32_t CellId;

struct Modifications {
  std::vector<std::pair<CellId, CellId>> eaten;
  std::vector<Cell *> added;
  std::set<Cell *> updated;
};

struct Top {
  static constexpr unsigned maxlen = 10;
  unsigned len;
  Player *players[maxlen];
  void reset();
  void add(Player *);
};

struct Cell : Item {
  // Static parameters
  Game &game;
  enum Type { PLAYER, VIRUS, PELLET, FOOD, size } type;
  uint32_t color = 0xFFFFFF;
  CellId id = 1;
  std::u16string name;

  // Dynamic parameters (mass, radius)
  virtual void setMass(unsigned);
  unsigned mass;
  double r;
  // Dynamic parameters (other)
  Vec2 pos, velocity, collideVelocity;

  // State changes flags
  bool updated = false;
  unsigned newMass;
  bool eaten = false;

  Cell(Game &);
  virtual ~Cell() {}
  Aabb getAabb() const override;
  Aabb getPotentialAabb() const override;
  virtual void step();
};

struct PlayerCell : Cell {
  Player *player;
  bool exploded = false;
  Vec2 target;
  bool targetEnabled = false;

  PlayerCell(Game &, Player *p);
  ~PlayerCell() override;
  void setMass(unsigned) override;
  void step() override;
  PlayerCell *split(Vec2 impulse);
};

struct Virus : Cell {
  Virus(Game &);
};

struct Pellet : Cell {
  Pellet(Game &);
};

struct FoodCell : Cell {
  FoodCell(Game &);
};

struct Game {
  Aabb size;
  std::set<Player *> players;
  Modifications mod;
  Top top;

  Game();
  void joinPlayer(Player *player);
  void step();
  void stop();

 protected:
  Broadphase b;
  CellId cellId;
  std::random_device rd;
  std::set<Cell *> cells;
  std::set<Cell *> inactiveCells;

  Vec2 randomPoint() const;
  unsigned cellCountByType[Cell::Type::size] = {0};
  void handleInteraction(Cell *fst, Cell *snd);

  friend class Cell;
};
