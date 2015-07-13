#pragma once

#include <cstdint>
#include <vector>

struct Game;
struct OutputEventBuffer {
  OutputEventBuffer(Game &game);
  const std::vector<char> &fieldSize();
  const std::vector<char> &reset();
  const std::vector<char> &ownsBlob(uint32_t x);
  const std::vector<char> &fullWorld();
  const std::vector<char> &modifyWorld(const struct Player *p);
  const std::vector<char> &top();

 protected:
  Game &game;
  std::vector<char> out;
};
