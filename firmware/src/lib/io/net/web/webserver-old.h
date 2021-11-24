#ifndef _webserver_INCLUDED_
#define _webserver_INCLUDED_

#include "data_type.h"
#include "mongoose.h"

data_type(
    {
      char port[32];
    },
    WebServerConfig_t);

typedef void (*HandlerCallback_t)(struct mg_connection *nc, struct http_message *hm);
typedef bool (*WSHandlerCallback_t)(cJSON *req, cJSON *resp);

typedef enum
{
  HTTP_GET = 1,
  HTTP_POST = 2
} HTTPRequest_t;

struct WebRequestHandler
{
  char *path;
  int request;
  HandlerCallback_t callback;
  struct WebRequestHandler *next;
};

struct WebSocketDataHandler
{
  char *command;
  WSHandlerCallback_t callback;
  struct WebSocketDataHandler *next;
};

typedef struct WebRequestHandler WebRequestHandler_t;
typedef struct WebSocketDataHandler WebSocketDataHandler_t;

int webserverWSRegister(WebSocketDataHandler_t *handler);
int webserverRegister(WebRequestHandler_t *handler);
int webserverInit(WebServerConfig_t *config);
void webServerBroadcastData(uint8_t *data, int len);
void webServerSendJson(struct mg_connection *c, cJSON *json);
void webServerBroadcastJson(cJSON *json);

#endif