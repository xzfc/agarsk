#pragma once

#include <cstring>
#include <locale>
#include <codecvt>

struct BytesOut {
  const char *s;
  size_t len;
  size_t pos;

  uint8_t getByte()
  { if(pos == len) throw 0;
    return s[pos++]; }

  template <class T>
  T get()
  { union
    { unsigned char data[sizeof(T)];
      T result; };
    for (int i = sizeof(T)-1; i >= 0; i--)
      data[i] = getByte();
    return result; }

  std::u16string getU16String()
  { return std::u16string(); }
};
