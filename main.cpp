#include "game.hpp"
#include "ws.hpp"
#include "outputEvent.hpp"

#include <iostream>

int main() {
  std::unique_ptr<IWsServer> ws(wsServer(8000));
  Game game;
  game.addPellets(200);
  ws->run();
  std::cout << "Game started!\n";
  BytesOut b;
  for (;;) {
    {
      auto input = ws->getInput();
      for (auto & event : *input)
        event->apply(game);
    }

    game.step();
    
    for (auto player : game.players) {
      Reset(game, b);
      player->connection->send(b.out);
      
      FullWorld(game, b);
      player->connection->send(b.out);

      ModifyWorld(game, b);
      player->connection->send(b.out);

      for (auto newCell : player->newCells) {
        b.clear();
        b.put<uint8_t>(32);
        b.put<uint32_t>(newCell);
        player->connection->send(b.out);
      }
    }

    if(system("sleep 0.01"))
      break;
  }
}
