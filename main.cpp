#include "game.hpp"
#include "ws.hpp"

int main() {
  WsServer ws(8000);
  Game game;
  ws.run();

  
}

int main_old() {
  WsServer ws(8000);
  Game game;

  Player players[3];
  for (auto &p : players)
    game.joinPlayer(&p);
  //players[0].cells[0]->pos = {0,0};
  //players[1].cells[0]->pos = {100,0};
  players[0].target = {100, 0};

  game.addPellets(2000);
  char fname[32];
  
  for (int i = 0; i < 1000 || 1; i++) {
    sprintf(fname, "out/%04d.svg", i);
    //if(i % 10 == 0) game.svg(fname);
    game.step();
  }
  
  game.stop();
}
