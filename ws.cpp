#include "ws.hpp"

#include <thread>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
#include <server_ws.hpp>

typedef SimpleWeb::SocketServer<SimpleWeb::WS> WS;
typedef std::shared_ptr<WS::Connection> pConnection;

struct WsConnection : IWsConnection {
  struct WsServer &serv;
  pConnection con;

  WsConnection(WsServer &, pConnection);
  
  void send(const std::vector<char> &msg) override;
  ~WsConnection() override {}
};

struct WsServer : IWsServer {
  WsServer(unsigned port);
  ~WsServer() override;
  void run() override;
  void stop() override;
  std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> getInput() override;

  bool running = false;
  WS ws;
  std::thread serverThread;

  boost::shared_mutex idMutex;
  std::map<pConnection, Player*> connection2id;

  std::mutex inputMutex;
  std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> input;

  void addInput(std::unique_ptr<InputEvent> &e) {
    std::lock_guard<std::mutex> _(inputMutex);
    input->push_back(std::move(e));
  }
};

WsConnection::WsConnection(WsServer &serv, pConnection con)
    : serv(serv), con(con)
{}

void WsConnection::send(const std::vector<char> &msg) {
  std::stringstream ss;
  ss.write(msg.data(), msg.size());
  serv.ws.send(con, ss, nullptr, 130);
}

WsServer::WsServer(unsigned port)
    : ws(port, 4 /* threads */)
{
  input.reset(new std::vector<std::unique_ptr<InputEvent>>);
  
  auto &endpoint = ws.endpoint["/"];
  
  endpoint.onopen = [&](pConnection connection) {
    auto player = new Player;
    idMutex.lock();
    connection2id[connection] = player;
    idMutex.unlock();

    player->connection = new WsConnection(*this, connection);

    std::unique_ptr<InputEvent> ev(new Connect);
    ev->player = player;
    addInput(ev);
  };

  endpoint.onclose = [&](pConnection connection, int status, const std::string& reason) {
    idMutex.lock();
    auto player = connection2id.at(connection);
    connection2id.erase(connection);
    idMutex.unlock();

    std::unique_ptr<InputEvent> ev(new Disconnect);
    ev->player = player;
    addInput(ev);
  };

  endpoint.onerror = [&](pConnection connection, const boost::system::error_code& ec) {
    endpoint.onclose(connection, 0, "");
  };

  endpoint.onmessage = [&](pConnection connection, std::shared_ptr<WS::Message> message) {
    idMutex.lock_shared();
    auto player = connection2id.at(connection);
    idMutex.unlock_shared();

    std::stringstream ss;
    message->data >> ss.rdbuf();
    std::string s = ss.str();
    
    std::unique_ptr<InputEvent> ev(InputEvent::parse(s.c_str(), s.length()));
    ev->player = player;
    addInput(ev);
  };
}

WsServer::~WsServer() {
  stop();
}

void WsServer::run() {
  if (running) return;
  running = true;
  serverThread = std::thread( [&]() { ws.start(); });
}

void WsServer::stop() {
  if (!running) return;
  running = false;
  ws.stop();
  serverThread.join();
}

std::unique_ptr<std::vector<std::unique_ptr<InputEvent>>> WsServer::getInput() {
  inputMutex.lock();
  auto result = std::move(input);
  input.reset(new std::vector<std::unique_ptr<InputEvent>>);
  inputMutex.unlock();
  return result;
}

IWsServer *wsServer(unsigned port) {
  return new WsServer(port);
}
