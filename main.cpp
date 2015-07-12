#include "game.hpp"
#include "ws.hpp"
#include "outputEvent.hpp"

#include <iostream>
#include <chrono>
#include <thread>

struct Ticker {
  Ticker(unsigned tpsTarget);
  bool tick();
  double ticksPerSecond();

  typedef std::chrono::steady_clock clock_t;
  unsigned long iteration;
  bool first;
  clock_t::duration period;
  clock_t::time_point start;
};

Ticker::Ticker(unsigned tpsTarget) {
  iteration = 0;
  using namespace std::literals::chrono_literals;
  period = 1s; period /= tpsTarget;
}

bool Ticker::tick() {
  if (iteration == 0) {
    start = clock_t::now();
    first = false;
    iteration++;
  } else {
    auto fromStart = clock_t::now() - start;
    if (fromStart / period < iteration-1) {
      iteration++;
      return true;
    }
    auto nextIterTime = (iteration++)*period;
    std::this_thread::sleep_for(nextIterTime - fromStart);
  }
  return false;
}

int main() {
  std::unique_ptr<IWsServer> ws(wsServer(8000));
  Game game;
  ws->run();
  std::cout << "Game started!\n";
  OutputEventBuffer b(game);
  Ticker ticker(25);
  for (;;) {
    ticker.tick();
    {
      auto input = ws->getInput();
      for (auto & event : *input)
        event->apply(game);
    }

    game.step();
    
    for (auto player : game.players) {
      player->connection->send(b.modifyWorld());

      for (auto newCell : player->newCells)
        player->connection->send(b.ownsBlob(newCell));
      player->newCells.clear();

      if (ticker.iteration % 32 == 0)
        player->connection->send(b.top());
    }
  }
}
