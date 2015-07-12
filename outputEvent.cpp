#include "outputEvent.hpp"

#include <cstring>

#include "game.hpp"

namespace {
struct Helper {
  std::vector<char> &out;
  
  Helper(std::vector<char> &out, uint8_t opcode)
      : out(out) {
    out.clear();
    out.push_back(opcode);
  }

  template <class T>
  void scalar(T x) {
    uint8_t s[sizeof(T)];
    std::memcpy(s, &x, sizeof(T));
    for (size_t i = 0; i < sizeof(T); i++)
      out.push_back(s[i]);
  }

  void string(const std::u16string str) {
    for (auto c: str)
      scalar<uint16_t>(c);
    scalar<int16_t>(0);
  }

  void cell(Cell *c) {
    scalar<uint32_t>(c->id);
    scalar<int16_t>(c->pos.x);
    scalar<int16_t>(c->pos.y);
    scalar<int16_t>(c->r);
    scalar<uint8_t>(c->color >>  0 & 0xFF);
    scalar<uint8_t>(c->color >>  8 & 0xFF);
    scalar<uint8_t>(c->color >> 16 & 0xFF);
    scalar<uint8_t>(c->type == Cell::VIRUS?1:0);
    string(c->name);
  }
};
}

OutputEventBuffer::OutputEventBuffer(Game &game)
    : game(game) {
}

const std::vector<char> &OutputEventBuffer::fieldSize() {
  Helper b(out, 64);
  b.scalar<double>(game.size.x0);
  b.scalar<double>(game.size.y0);
  b.scalar<double>(game.size.x1);
  b.scalar<double>(game.size.y1);
  return out;
}

const std::vector<char> &OutputEventBuffer::reset() {
  Helper b(out, 20);
  return out;
}

const std::vector<char> &OutputEventBuffer::ownsBlob(uint32_t newCell) {
  Helper b(out, 32);
  b.scalar<uint32_t>(newCell);
  return out;
}

const std::vector<char> &OutputEventBuffer::fullWorld() {
  Helper b(out, 16);
  
  b.scalar<uint16_t>(0);
  
  for (auto &c : game.cells)
    b.cell(c);
  b.scalar<uint32_t>(0);
  
  b.scalar<uint32_t>(0);

  return out;
};

const std::vector<char> &OutputEventBuffer::modifyWorld() {
  Helper b(out, 16);

  b.scalar<uint16_t>(game.mod.eaten.size());
  for (auto &e : game.mod.eaten) {
    b.scalar<uint32_t>(e.first);
    b.scalar<uint32_t>(e.second);
  }

  for (auto &c : game.cells)
    if (c->updated && !c->eaten)
      b.cell(c);

  b.scalar<uint32_t>(0);
  
  b.scalar<uint32_t>(0);

  return out;
}

const std::vector<char> &OutputEventBuffer::top() {
  Helper b(out, 49);
  
  b.scalar<uint32_t> (game.top.len);
  for (size_t i = 0; i < game.top.len; i++) {
    CellId minId = 0;
    for (auto cell : game.top.players[i]->cells)
      if (!minId || minId > cell->id)
        minId = cell->id;
    b.scalar<uint32_t>(minId);
    b.string(game.top.players[i]->name);
  }

  return out;
}
