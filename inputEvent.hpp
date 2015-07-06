#pragma once
#include <cstddef> // size_t

struct InputEvent {  
  struct Player *player = 0;
  virtual void apply(struct Game *) {}
  static InputEvent *parse(const char *, size_t);
};

struct Connect : InputEvent {
  Connect() {}
};

struct Disconnect : InputEvent {
  Disconnect() {}
};
