#pragma once

#include <sstream>
#include <iostream>

// ws: accept connection -> create new player
// ws: receive message -> modify player state
// ws: disconnect -> modify player state; remove player
// game: send (state)

struct Player;
struct WsServer {
  WsServer(unsigned port);
  ~WsServer();
  void wsRun();
  void wsStop();
  
  virtual Player* wsOnConnect() = 0;
  virtual void wsOnReceive(Player *, const char *data, size_t len) = 0;
  virtual void wsOnDisconnect(Player *) = 0;
  
  void wsSend(Player *, std::stringstream msg);
 private:
  struct WsServerInternals *priv;
};
