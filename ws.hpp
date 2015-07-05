#pragma once

#include <sstream>
#include <iostream>

// ws: accept connection -> create new player
// ws: receive message -> modify player state
// ws: disconnect -> modify player state; remove player
// game: send (state)

struct WsServer {
  WsServer(unsigned port);
  ~WsServer();
  void wsRun();
  void wsStop();
  
  virtual void wsOnConnect(unsigned id) = 0;
  virtual void wsOnReceive(unsigned id, std::istream &ss) = 0;
  virtual void wsOnDisconnect(unsigned id) = 0;
  
  void wsSend(unsigned id, std::stringstream msg);
 private:
  struct WsServerInternals *priv;
};
