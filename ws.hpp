#pragma once
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

#include "player.hpp"
#include "inputEvent.hpp"

struct Update;

struct IWsConnection {
  virtual void send(const std::vector<char> &msg) = 0;
  virtual ~IWsConnection() {};
};

struct IWsServer {
  virtual ~IWsServer() {};
  virtual void run() = 0;
  virtual void stop() = 0;
  virtual std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> getInput() = 0;
  //void send(Player*, const std::vector<char> &msg);
};

IWsServer *wsServer(unsigned port);
