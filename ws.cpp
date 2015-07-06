#include "ws.hpp"

#include <thread>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <server_ws.hpp>

typedef SimpleWeb::SocketServer<SimpleWeb::WS> WS;
typedef std::shared_ptr<WS::Connection> pConnection;
struct WsConnection { pConnection c; };

struct WsServerInternals {
  bool running = false;
  WS ws;
  std::thread serverThread;

  boost::shared_mutex idMutex;
  std::map<Player*, pConnection> id2connection;
  std::map<pConnection, Player*> connection2id;

  std::mutex inputMutex;
  std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> input;
  void addInput(std::unique_ptr<InputEvent> &e) {
    std::lock_guard<std::mutex> _(inputMutex);
    input->push_back(std::move(e));
  }
  
  WsServerInternals(int port) : ws(port, 4 /* threads */) {
    input.reset(new std::vector<std::unique_ptr<InputEvent>>);
  }
};

WsServer::WsServer(unsigned port) {
  priv = new WsServerInternals(port);
  
  auto &endpoint = priv->ws.endpoint["/"];
  
  endpoint.onopen = [&](pConnection connection) {
    auto player = new Player;
    priv->idMutex.lock();
    priv->id2connection[player] = connection;
    priv->connection2id[connection] = player;
    priv->idMutex.unlock();

    std::unique_ptr<InputEvent> ev(new Connect);
    ev->player = player;
    priv->addInput(ev);
  };

  endpoint.onclose = [&](pConnection connection, int status, const std::string& reason) {
    priv->idMutex.lock();
    auto player = priv->connection2id.at(connection);
    priv->id2connection.erase(player);
    priv->connection2id.erase(connection);
    priv->idMutex.unlock();

    std::unique_ptr<InputEvent> ev(new Disconnect);
    ev->player = player;
    priv->addInput(ev);
    // TODO: delete player here or make game delete him
  };

  endpoint.onerror = [&](pConnection connection, const boost::system::error_code& ec) {
    endpoint.onclose(connection, 0, "");
  };

  endpoint.onmessage = [&](pConnection connection, std::shared_ptr<WS::Message> message) {
    priv->idMutex.lock_shared();
    auto player = priv->connection2id.at(connection);
    priv->idMutex.unlock_shared();

    std::stringstream ss;
    message->data >> ss.rdbuf();
    std::string s = ss.str();
    
    std::unique_ptr<InputEvent> ev(InputEvent::parse(s.c_str(), s.length()));
    ev->player = player;
    priv->addInput(ev);
  };
}

WsServer::~WsServer() {
  stop();
  delete priv;
}

void WsServer::run() {
  if (priv->running) return;
  priv->running = true;
  priv->serverThread = std::thread( [&]() { priv->ws.start(); });
}

void WsServer::stop() {
  if (!priv->running) return;
  priv->running = false;
  priv->ws.stop();
  priv->serverThread.join();
}

std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> WsServer::getInput() {
  priv->inputMutex.lock();
  auto result = std::move(priv->input);
  priv->input.reset(new std::vector<std::unique_ptr<InputEvent>>);
  priv->inputMutex.unlock();
  return result;
}

void WsServer::send(Player *player, const std::vector<char> &msg) {
  pConnection connection;
  {
    boost::shared_lock<boost::shared_mutex>(priv->idMutex);
    auto iter = priv->id2connection.find(player);
    if (iter == priv->id2connection.end())
      return;
    connection = iter->second;
  }
  std::stringstream ss;
  ss.write(msg.data(), msg.size());
  priv->ws.send(connection, ss, nullptr, 130);
}
