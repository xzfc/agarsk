#include "inputEvent.hpp"
#include "bytes.hpp"

struct Spawn : InputEvent {
  std::u16string name;
  Spawn(BytesOut &b)
  { name = b.getU16String(); }
};

struct Join : InputEvent {
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

static InputEvent *parse_(BytesOut &b) {
  switch (b.getByte()) {
    case   0: return new Spawn(b);
    case   1: return new Join;
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
    return new Error;
  }
  return ie;
}
