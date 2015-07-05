#include "ws.hpp"

#include <thread>
#include <boost/thread/shared_mutex.hpp>
#include <server_ws.hpp>

typedef SimpleWeb::SocketServer<SimpleWeb::WS> WS;
typedef std::shared_ptr<WS::Connection> pConnection;

struct WsServerInternals {
  bool running = false;
  WS ws;
  std::thread serverThread;

  boost::shared_mutex idMutex;
  unsigned idCounter = 0;
  std::map<unsigned, pConnection> id2connection;
  std::map<pConnection, unsigned> connection2id;
  
  WsServerInternals(int port) : ws(port, 4 /* threads */) {}
};

WsServer::WsServer(unsigned port) {
  priv = new WsServerInternals(port);
  
  auto &endpoint = priv->ws.endpoint["/"];
  
  endpoint.onopen = [&](pConnection connection) {
    priv->idMutex.lock();
    auto id = ++priv->idCounter;
    priv->id2connection[id] = connection;
    priv->connection2id[connection] = id;
    priv->idMutex.unlock();
    wsOnConnect(id);
  };

  endpoint.onclose = [&](pConnection connection, int status, const std::string& reason) {
    priv->idMutex.lock();
    auto id = priv->connection2id.at(connection);
    priv->id2connection.erase(id);
    priv->connection2id.erase(connection);
    priv->idMutex.unlock();
    wsOnDisconnect(id);
  };

  endpoint.onerror = [&](pConnection connection, const boost::system::error_code& ec) {
    endpoint.onclose(connection, 0, "");
  };

  endpoint.onmessage = [&](pConnection connection, std::shared_ptr<WS::Message> message) {
    priv->idMutex.lock_shared();
    auto id = priv->connection2id.at(connection);
    priv->idMutex.unlock_shared();
    wsOnReceive(id, message->data);
  };
}

WsServer::~WsServer() {
  wsStop();
  delete priv;
}

void WsServer::wsRun() {
  if (priv->running) return;
  priv->serverThread = std::thread( [&]() { priv->ws.start(); });
}

void WsServer::wsStop() {
  if (!priv->running) return;
  priv->ws.stop();
  priv->serverThread.join();
}

void WsServer::wsSend(unsigned id, std::stringstream msg) {
  pConnection connection;
  {
    boost::shared_lock<boost::shared_mutex>(priv->idMutex);
    auto iter = priv->id2connection.find(id);
    if (iter == priv->id2connection.end())
      return;
    connection = iter->second;
  }
  priv->ws.send(connection, msg, nullptr, 130);
}
