#pragma once

#include <cstddef> // size_t
#include <cstdint>
#include <vector>

struct WsSend {
  static constexpr size_t max_length = 1024*1024;
  unsigned char * const data;
  size_t position, length;

  ~WsSend();
  
  void writeRaw(const void*, size_t);
  void writeByte(uint8_t x)
  { data[position++] = x; }
  
  void write(uint8_t x)
  { writeByte(x); }
  
  void write(uint16_t x)
  { writeByte((x>>8)  & 0xFF);
    writeByte((x>>0)  & 0xFF); }
  
  void write(uint32_t x)
  { writeByte((x>>24) & 0xFF);
    writeByte((x>>16) & 0xFF);
    writeByte((x>>8)  & 0xFF);
    writeByte((x>>0)  & 0xFF); }

  void write(uint64_t x)
  { writeByte((x>>56) & 0xFF);
    writeByte((x>>48) & 0xFF);
    writeByte((x>>40) & 0xFF);
    writeByte((x>>32) & 0xFF);
    writeByte((x>>24) & 0xFF);
    writeByte((x>>16) & 0xFF);
    writeByte((x>>8)  & 0xFF);
    writeByte((x>>0)  & 0xFF); }

  void write(float x)
  {}
  
  void write(double x)
  {}

 protected:
  WsSend();
};

//struct 

struct WsServer {
  WsServer(int port);
  ~WsServer();
  
  struct libwebsocket_context *context;
  
  std::vector<struct WsClientData *> clients;
};
