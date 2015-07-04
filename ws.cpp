#include "ws.hpp"

#include <libwebsockets.h>

#include <iostream>
#include <cstring>
#include <cstdlib>

struct WsClientData {
  struct libwebsocket *ws;
  bool hello = false;

  bool bad = false;
};

static WsServer *server;

static int callback_http(struct libwebsocket_context *,
                         struct libwebsocket *,
                         enum libwebsocket_callback_reasons reason, void *user,
                         void *in, size_t len) {
    return 0;
}

static int callback_normal(struct libwebsocket_context *,
                           struct libwebsocket *ws,
                           enum libwebsocket_callback_reasons reason, void *user_,
                           void *in_, size_t len) {
  if(reason == 30) return 0;
  std::cout << "Callback!" << reason << "\n";
  WsClientData *user = static_cast<WsClientData *>(user_);
  uint8_t *in = static_cast<uint8_t*>(in_);
  switch (reason) {
    case LWS_CALLBACK_ESTABLISHED: {
      std::cout << "Connected!\n";
      new (user_) WsClientData;
      user->ws = ws;
      break;
    }
    case LWS_CALLBACK_RECEIVE: {
      std::cout << "Receive:";
      for (size_t i = 0; i < len; i++)
        std::cout << ' ' << (int)in[i];
      std::cout << " | ";
      for (size_t i = 0; i < len; i++)
        if ( ((char)in[i]) >= ' ' && ((char)in[i]) <= '~')
          std::cout << (char)in[i];
        else
          std::cout << '?';
      std::cout << "\n";
      
      break; 
      if (user->hello) {
        
      } else {
        if (len != 5 || in[0] != 255)
          user->bad = true;
        else
          user->hello = true;
      }
      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING];
      break;
    }
    case LWS_CALLBACK_WSI_DESTROY: {
      user->~WsClientData();
      break;
    }
  }
  return 0;
}

static struct libwebsocket_protocols protocols[] = {
  // { "http-only", callback_http,   0, 0, },
  { "nope",          callback_normal, sizeof (WsClientData), 1024*1024, },
  { NULL, NULL, 0, 0 }
};

WsServer::WsServer(int port) {
  server = this;
  
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof info);
  info.port = port;
  info.iface = NULL;
  info.extensions = libwebsocket_get_internal_extensions();
  info.uid = info.gid = -1;
  // info.opts = 0; // wtf is this?
  // info.ka_time = 5; // seconds or milliseconds or anything else?
  info.protocols = protocols;

  context = libwebsocket_create_context(&info);
}

WsServer::~WsServer() {
  //force_exit = 1;
  libwebsocket_cancel_service(context);
  libwebsocket_context_destroy(context);
}

int main() {
  WsServer w(8000);
  while (1) {
    libwebsocket_service(w.context, 50);
  }
}
