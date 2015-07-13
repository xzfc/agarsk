#pragma once
#include <cstddef>  // size_t

struct InputEvent {
  struct Player *player = 0;
  virtual void apply(struct Game &) {}
  virtual ~InputEvent() {}
  static InputEvent *parse(const char *, size_t);
};

struct Connect : InputEvent {
  Connect() {}
  void apply(struct Game &) override;
};

struct Disconnect : InputEvent {
  Disconnect() {}
  void apply(struct Game &) override;
};
