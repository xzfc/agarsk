#include "inputEvent.hpp"
#include "game.hpp"
#include "ws.hpp" // IWsConnection::send
#include "outputEvent.hpp" // BytesOut

#include <iostream>
#include <locale>
#include <codecvt>

struct BytesIn {
  const char *s;
  size_t len;
  size_t pos;

  uint8_t getByte()
  { if(pos == len) throw 0;
    return s[pos++]; }

  template <class T>
  T get() {
    union {
      unsigned char data[sizeof(T)];
      T result;
    };
    for (int i = 0; i<sizeof(T); i++)
      data[i] = getByte();
    return result;
  }

  std::u16string getU16String(bool nullTerminated) {
    std::u16string res;
    for (;;) {
      if (len == pos && !nullTerminated)
        return res;
      auto c = get<uint16_t>();
      if (!c && nullTerminated)
        return res;
      res += c;
    }
  }
};

struct Spawn : InputEvent {
  std::u16string name;
  Spawn(BytesIn &b)
  { name = b.getU16String(false); }

  void apply(Game &g) override {
    player->name = name; // TODO: change only on death
    g.joinPlayer(player); // TODO: merge apply and joinPlayer methods
  }
};

struct Spectrate : InputEvent {
};

struct Direction : InputEvent {
  double x, y;
  uint32_t width;
  Direction(BytesIn &b) {
    x = b.get<double>();
    y = b.get<double>();
    width = b.get<uint32_t>();
  }
  void apply(Game &g) override {
    player->target = {x, y};
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
  Token(BytesIn &b) {
    b.pos = b.len; /* just ignore rest bytes */
  }
};

struct Hello254 : InputEvent {
  uint32_t value4;
  Hello254(BytesIn &b)
  { value4 = b.get<uint32_t>(); }
};

struct Hello255 : InputEvent {
  uint32_t value154669603;
  Hello255(BytesIn &b)
  { value154669603 = b.get<uint32_t>(); }
};

struct Error : InputEvent {
};

void Connect::apply(Game &game) {
  game.players.insert(player);
  BytesOut b;
  
  FieldSize(game, b);
  player->connection->send(b.out);

  Reset(game, b);
  player->connection->send(b.out);

  FullWorld(game, b);
  player->connection->send(b.out);
}

void Disconnect::apply(Game &game) {
  game.players.erase(player);
  for (auto cell : player->cells)
    cell->player = nullptr;
  delete player;
  player = 0;
}

static InputEvent *parse_(BytesIn &b) {
  auto x = b.getByte();
  //std::cout << "Input event " << (unsigned)x << "\n";
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
    case 254: return new Hello254(b);
    case 255: return new Hello255(b);
  }
  return 0;
}

InputEvent *InputEvent::parse(const char *data, size_t len) {
  BytesIn b {data, len, 0};
  /*
  std::cout << "Input event:";
  for (size_t i = 0; i < len; i++) std::cout << ' ' << (unsigned)(unsigned char)data[i];
  std::cout << "\n";*/

  InputEvent *ie = 0;
  try { ie = parse_(b); }
  catch (int x) { std::cout << "Input event error 1\n"; };
  if (!ie || b.len != b.pos) {
    delete ie;
    std::cout << "Input event error 2\n";
    return new Error;
  }
  return ie;
}
