#pragma once
#include "game.hpp"
#include <vector>
#include <cstring>

struct BytesOut {
  std::vector<char> out;
  std::stringstream ss;

  std::stringstream &getSs() {
    ss.write(out.data(), out.size());
    return ss;
  }
  
  void clear() {
    ss.clear();
    out.clear();
  }
  
  void put(uint8_t x)
  { out.push_back(x); }

  template <class T>
  void put(T x) {
    uint8_t s[sizeof(T)];
    std::memcpy(s, &x, sizeof(T));
    for (int i = 0; i < sizeof(T); i++)
      put(s[i]);
  }
};

void Yoba(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(64);
  b.put<double>(0);
  b.put<double>(0);
  b.put<double>(1000.0);
  b.put<double>(1000.0);
}

void FullWorld(Game &game, BytesOut &b) {
  b.clear();
  b.put<uint8_t>(16);
  b.put<uint16_t>(0);
  for (auto & c : game.cells) {
    b.put<uint32_t>(c->id);
    b.put<int16_t>(c->pos.x);
    b.put<int16_t>(c->pos.y);
    b.put<int16_t>(c->r);
    b.put<uint8_t>(0x79); // r
    b.put<uint8_t>(0xff); // g
    b.put<uint8_t>(0x07); // b
    b.put<uint8_t>(0); // flags
    b.put<uint16_t>(0); // name
  }
  b.put<uint32_t>(0);
};
