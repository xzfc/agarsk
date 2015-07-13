#pragma once
#include <set>
#include <string>
#include <vector>

#include "bullet.hpp"  // Vec2

struct Player {
  unsigned id;

  unsigned totalMass = 0;

  Vec2 target;
  bool shoot = false;
  bool split = false;

  enum class Mode { DEFAULT, SPECTRATE, GAME } mode = Mode::DEFAULT;
  std::set<struct PlayerCell *> cells;
  std::u16string name;

  std::vector<uint32_t> newCells;
  // If visibleSwap is false, then visible0 represents current frame, and
  // visible1 represents previous. If visibleSwap is true, roles of visible0
  // and visible1 is swapped.
  bool visibleSwap = false;
  std::set<uint32_t> visible0, visible1;

  struct IWsConnection *connection;
};
