#pragma once
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

#include "player.hpp"
#include "inputEvent.hpp"

struct Update;

struct WsServer {
  WsServer(unsigned port);
  ~WsServer();
  void run();
  void stop();
  std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> getInput();
  void send(Player*, const std::vector<char> &msg);
 private:
  struct WsServerInternals *priv;
};
