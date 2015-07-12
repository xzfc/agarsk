#pragma once
#include "game.hpp"
#include <vector>
#include <cstring>

struct OutputEventBuffer {
  Game &game;
  std::vector<char> out;
  
  OutputEventBuffer(Game &game);
  
  void putByte(uint8_t x)
  { out.push_back(x); }

  template <class T>
  void put(T x) {
    uint8_t s[sizeof(T)];
    std::memcpy(s, &x, sizeof(T));
    for (int i = 0; i < sizeof(T); i++)
      putByte(s[i]);
  }

  void putString(const std::u16string str) {
    for (auto c: str)
      put<uint16_t>(c);
    put<int16_t>(0);
  }

  void start(uint8_t opcode) {
    out.clear();
    put<uint8_t>(opcode);
  }

  const std::vector<char> &_fieldSize();
  const std::vector<char> &_reset();
  const std::vector<char> &_ownsBlob(uint32_t);

  void __cell(Cell *);

  const std::vector<char> &_fullWorld();
  const std::vector<char> &_modifyWorld();
  const std::vector<char> &_top();
};

inline OutputEventBuffer::OutputEventBuffer(Game &game) : game(game)
{ }

inline const std::vector<char> &OutputEventBuffer::_fieldSize() {
  start(64);
  
  put<double>(game.size.x0);
  put<double>(game.size.y0);
  put<double>(game.size.x1);
  put<double>(game.size.y1);

  return out;
}

inline const std::vector<char> &OutputEventBuffer::_reset() {
  start(20);
  return out;
}

inline void OutputEventBuffer::__cell(Cell *c) {
  put<uint32_t>(c->id);
  put<int16_t>(c->pos.x);
  put<int16_t>(c->pos.y);
  put<int16_t>(c->r);
  put<uint8_t>(c->color >>  0 & 0xFF);
  put<uint8_t>(c->color >>  8 & 0xFF);
  put<uint8_t>(c->color >> 16 & 0xFF);
  put<uint8_t>(c->type == Cell::VIRUS?1:0);
  putString(c->name);
}

inline const std::vector<char> &OutputEventBuffer::_ownsBlob(uint32_t newCell) {
  start(32);
  put<uint32_t>(newCell);

  return out;
}

inline const std::vector<char> &OutputEventBuffer::_fullWorld() {
  start(16);
  
  put<uint16_t>(0);
  
  for (auto &c : game.cells)
    __cell(c);
  put<uint32_t>(0);
  
  put<uint32_t>(0);

  return out;
};

inline const std::vector<char> &OutputEventBuffer::_modifyWorld() {
  start(16);

  put<uint16_t>(game.mod.eaten.size());
  for (auto &e : game.mod.eaten) {
    put<uint32_t>(e.first);
    put<uint32_t>(e.second);
  }

  for (auto &c : game.cells)
    if (c->updated && !c->eaten)
      __cell(c);
  
  put<uint32_t>(0);
  
  put<uint32_t>(0);

  return out;
}

inline const std::vector<char> &OutputEventBuffer::_top() {
  start(49);
  
  put<uint32_t> (game.top.len);
  for (size_t i = 0; i < game.top.len; i++) {
    CellId minId = 0;
    for (auto cell : game.top.players[i]->cells)
      if (!minId || minId > cell->id)
        minId = cell->id;
    put<uint32_t>(minId);
    putString(game.top.players[i]->name);
  }

  return out;
}
