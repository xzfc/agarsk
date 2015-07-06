#pragma once
#include <set>
#include <string>

#include "bullet.hpp" // Vec2

struct Player {
  unsigned id;
  Vec2 target;
  bool shoot = false;
  bool split = false;

  enum class Mode {
    DEFAULT, SPECTRATE, GAME
  } mode;

  std::set<struct PlayerCell*> cells;
  std::u16string name;
  uint64_t color = 0;
};
