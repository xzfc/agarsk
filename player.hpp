#pragma once
#include <set>
#include <string>

#include "bullet.hpp" // Vec2
#include <iostream>
struct Player {
  unsigned id;
  
  Vec2 target;
  bool shoot = false;
  bool split = false;
  
  enum class Mode { DEFAULT, SPECTRATE, GAME }
    mode = Mode::DEFAULT;
  std::set<struct PlayerCell*> cells;
  std::u16string name;

  std::vector<uint32_t> newCells;

  struct IWsConnection *connection;
};
