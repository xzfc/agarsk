#pragma once
#include "game.hpp"
#include <vector>
#include <cstring>

struct BytesOut {
  std::vector<char> out;

  void clear() {
    out.clear();
  }
  
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
};

inline void FieldSize(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(64);
  b.put<double>(game.size.x0);
  b.put<double>(game.size.y0);
  b.put<double>(game.size.x1);
  b.put<double>(game.size.y1);
}

inline void Reset(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(20);
}

inline void Cell_(Cell *c, BytesOut &b) {
  b.put<uint32_t>(c->id);
  b.put<int16_t>(c->pos.x);
  b.put<int16_t>(c->pos.y);
  b.put<int16_t>(c->r);
  b.put<uint8_t>(c->color >>  0 & 0xFF); // r
  b.put<uint8_t>(c->color >>  8 & 0xFF); // g
  b.put<uint8_t>(c->color >> 16 & 0xFF); // b
  b.put<uint8_t>(0); // flags
  b.putString(c->name); // name
}

inline void FullWorld(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(16);
  
  b.put<uint16_t>(0);
  
  for (auto &c : game.cells)
    Cell_(c, b);
  b.put<uint32_t>(0);
  
  b.put<uint32_t>(0);
};

inline void ModifyWorld(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(16);

  b.put<uint16_t>(game.mod.eaten.size());
  for (auto &e : game.mod.eaten) {
    b.put<uint32_t>(e.first->id);
    b.put<uint32_t>(e.second->id);
  }

  for (auto &c : game.cells)
    if (c->moved && !c->eaten)
      Cell_(c, b);
  
  b.put<uint32_t>(0);
  
  b.put<uint32_t>(0);
}
