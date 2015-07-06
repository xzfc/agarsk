#include "inputEvent.hpp"
#include "game.hpp"
#include "bytes.hpp"

#include <iostream>

struct Spawn : InputEvent {
  std::u16string name;
  Spawn(BytesOut &b)
  { name = b.getU16String(); }

  void apply(Game &g) override {
    std::cout << "Yay, player joined!\n";
    g.joinPlayer(player);
  }
};

struct Spectrate : InputEvent {
};

struct Direction : InputEvent {
  double x, y;
  uint32_t width;
  Direction(BytesOut &b) {
    x = b.get<double>();
    y = b.get<double>();
    width = b.get<uint32_t>();
  }
};

struct Split : InputEvent {
};

struct Eject : InputEvent {
};

struct AFK : InputEvent {
};

struct Explode : InputEvent {
};

struct Q : InputEvent {
};

struct Token : InputEvent {
  std::string token;
  Token(BytesOut &b) {
    b.pos = b.len; /* just ignore rest bytes */
  }
};

struct Error : InputEvent {
};

void Connect::apply(Game &game) {
  game.players.insert(player);
}

void Disconnect::apply(Game &game) {
  game.players.erase(player);
}

static InputEvent *parse_(BytesOut &b) {
  auto x = b.getByte();
  std::cout << "Input event " << (unsigned)x << "\n";
  switch (x) {
    case   0: return new Spawn(b);
    case   1: return new Spectrate;
    case  16: return new Direction(b);
    case  17: return new Split;
    case  18: return new Eject;
    case  19: return new AFK;
    case  20: return new Explode;
    case  21: return new Q;
    case  80: return new Token(b);
    case 254:
    case 255: return new Q(); // :3
  }
  return 0;
}

InputEvent *InputEvent::parse(const char *data, size_t len) {
  BytesOut b {data, len, 0};
  InputEvent *ie = parse_(b);
  if (!ie || b.len != b.pos) {
    delete ie;
    std::cout << "Input event error\n";
    return new Error;
  }
  return ie;
}
