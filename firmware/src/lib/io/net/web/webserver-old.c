#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mongoose.h"
#include "webserver.h"
#include "wifi_controller.h"

#include "data_type.h"

#define MIME_ENTRY(_ext, _type)   \
  {                               \
    _ext, sizeof(_ext) - 1, _type \
  }
static const struct
{
  const char *extension;
  size_t ext_len;
  const char *mime_type;
} mg_static_builtin_mime_types[] = {
    MIME_ENTRY("html", "text/html"),
    MIME_ENTRY("html", "text/html"),
    MIME_ENTRY("htm", "text/html"),
    MIME_ENTRY("shtm", "text/html"),
    MIME_ENTRY("shtml", "text/html"),
    MIME_ENTRY("css", "text/css"),
    MIME_ENTRY("js", "application/x-javascript"),
    MIME_ENTRY("ico", "image/x-icon"),
    MIME_ENTRY("gif", "image/gif"),
    MIME_ENTRY("jpg", "image/jpeg"),
    MIME_ENTRY("jpeg", "image/jpeg"),
    MIME_ENTRY("png", "image/png"),
    MIME_ENTRY("svg", "image/svg+xml"),
    MIME_ENTRY("txt", "text/plain"),
    MIME_ENTRY("torrent", "application/x-bittorrent"),
    MIME_ENTRY("wav", "audio/x-wav"),
    MIME_ENTRY("mp3", "audio/x-mp3"),
    MIME_ENTRY("mid", "audio/mid"),
    MIME_ENTRY("m3u", "audio/x-mpegurl"),
    MIME_ENTRY("ogg", "application/ogg"),
    MIME_ENTRY("ram", "audio/x-pn-realaudio"),
    MIME_ENTRY("xml", "text/xml"),
    MIME_ENTRY("ttf", "application/x-font-ttf"),
    MIME_ENTRY("json", "application/json"),
    MIME_ENTRY("xslt", "application/xml"),
    MIME_ENTRY("xsl", "application/xml"),
    MIME_ENTRY("ra", "audio/x-pn-realaudio"),
    MIME_ENTRY("doc", "application/msword"),
    MIME_ENTRY("exe", "application/octet-stream"),
    MIME_ENTRY("zip", "application/x-zip-compressed"),
    MIME_ENTRY("xls", "application/excel"),
    MIME_ENTRY("tgz", "application/x-tar-gz"),
    MIME_ENTRY("tar", "application/x-tar"),
    MIME_ENTRY("gz", "application/x-gunzip"),
    MIME_ENTRY("arj", "application/x-arj-compressed"),
    MIME_ENTRY("rar", "application/x-rar-compressed"),
    MIME_ENTRY("rtf", "application/rtf"),
    MIME_ENTRY("pdf", "application/pdf"),
    MIME_ENTRY("swf", "application/x-shockwave-flash"),
    MIME_ENTRY("mpg", "video/mpeg"),
    MIME_ENTRY("webm", "video/webm"),
    MIME_ENTRY("mpeg", "video/mpeg"),
    MIME_ENTRY("mov", "video/quicktime"),
    MIME_ENTRY("mp4", "video/mp4"),
    MIME_ENTRY("m4v", "video/x-m4v"),
    MIME_ENTRY("asf", "video/x-ms-asf"),
    MIME_ENTRY("avi", "video/x-msvideo"),
    MIME_ENTRY("bmp", "image/bmp"),
    MIME_ENTRY("woff2", "font/woff2"),
    {0, 0, 0}};

static portMUX_TYPE buf_spinlock = portMUX_INITIALIZER_UNLOCKED;

const char *getMimeType(const char *path)
{
  char *ext = strrchr(path, '.') + 1;
  if (ext == NULL)
    return NULL;

  for (int i = 0; mg_static_builtin_mime_types[i].extension != NULL; ++i)
  {

    if (strcasecmp(ext, mg_static_builtin_mime_types[i].extension) == 0)
    {
      printf("found mime: %s, for: %s\n", mg_static_builtin_mime_types[i].mime_type, ext);
      return mg_static_builtin_mime_types[i].mime_type;
    }
  }

  printf("unknown mine ext: %s\n", ext);

  return NULL;
}

//static char web_buffer[4096];
//static char web_send_buffer[8192];
static struct mg_str gzip_str;

struct mg_serve_http_opts opts;
      

typedef struct
{
  struct mg_connection *nc;
  char file[32];
} FileRequest_t;

typedef struct
{
  struct mg_mgr mgr;
  struct mg_connection *nc;
  QueueHandle_t http_serve_q;
  WebRequestHandler_t *headHandler;
  WebSocketDataHandler_t *headWSHandler;
} WebServerState_t;

static WebServerState_t state;
static WebServerConfig_t *config;

static bool ws_handle_json(cJSON *req, cJSON *resp)
{
  if (req == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      cJSON_AddStringToObject(resp, "error", error_ptr);
      printf("Error before: %s\n", error_ptr);
    }
    return true;
  }

  printf(cJSON_Print(req));
  printf("\n");
  cJSON *type = cJSON_GetObjectItem(req, "type");
  if (type == NULL)
  {
    cJSON_AddStringToObject(resp, "error", "'type' argument not found");
    return true;
  }

  cJSON *data = cJSON_GetObjectItem(req, "data");
  if (data == NULL)
  {
    cJSON_AddStringToObject(resp, "error", "'data' argument not found");
    return true;
  }

  WebSocketDataHandler_t *c = state.headWSHandler;
  while (c != NULL)
  {
    if (strcmp(type->valuestring, c->command) == 0)
    {
      printf("matched: %s, with: %s\n", type->string, c->command);
      if ((*c->callback)(data, resp))
        return true;
    }

    c = c->next;
  }

  if (c == NULL)
  {
    cJSON_AddStringToObject(resp, "type", "error");
    cJSON *errData = cJSON_AddObjectToObject(resp, "data");
    cJSON_AddStringToObject(errData, "id", 404);
    cJSON_AddStringToObject(errData, "error", "unable to find command");
    cJSON_AddStringToObject(errData, "command", type->valuestring);
    return true;
  }

  return false;
}

static void ws_frame_handler(
    struct mg_connection *nc,
    struct websocket_message *wm)
{
  cJSON *resp = cJSON_CreateObject();
  cJSON *req = cJSON_ParseWithOpts((const char *)wm->data, 0, 0);

  if (ws_handle_json(req, resp))
    webServerSendJson(nc, resp);

  cJSON_Delete(req);
  cJSON_Delete(resp);
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
  struct http_message *hm = (struct http_message *)ev_data;
  WebRequestHandler_t *c = state.headHandler;
  switch (ev)
  {
  case MG_EV_ACCEPT:
  {
    char addr[32];
    mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                        MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
    printf("Connection %p from %s\n", nc, addr);
    break;
  }

  case MG_EV_HTTP_REQUEST:
  {
    c = state.headHandler;

    while (c)
    {
      if (mg_vcmp(&hm->uri, c->path) == 0)
      {
        printf("matched: %s, with: %s\n", hm->uri.p, c->path);
        (*c->callback)(nc, hm);
        break;
      }

      c = c->next;
    }

    if (!c)
    {
      mg_serve_http(nc, hm, opts);
    }
    break;
  }

  case MG_EV_WEBSOCKET_FRAME:
  {
    struct websocket_message *wm = (struct websocket_message *)ev_data;

    ws_frame_handler(nc, wm);
    break;
  }

  case MG_EV_CLOSE:
    printf("Connection %p closed\n", nc);
    break;
  default:
    break;
  }
}

void serve_task(void *arg)
{
  mg_mgr_init(&state.mgr, NULL);
  state.nc = mg_bind(&state.mgr, &config->port[0], ev_handler);
  if (!state.nc)
  {
    printf("No connection from the mg_bind()\n");
  }

  printf("web server bound to port: %s\n", &config->port[0]);
  mg_set_protocol_http_websocket(state.nc);

  for (;;)
  {
    mg_mgr_poll(&state.mgr, 10);
    vTaskDelay(0);
  }
  mg_mgr_free(&state.mgr);
}

int webserverInit(WebServerConfig_t *_config)
{
  memset(&state, 0, sizeof(state));
  config = _config;

  opts.document_root = "/spiffs";

  state.http_serve_q = xQueueCreate(10, sizeof(FileRequest_t));
  xTaskCreate(serve_task, "serve_task", 8192, NULL, 0, NULL);

  gzip_str = mg_mk_str("gzip");

  return 0;
}

int webserverRegister(WebRequestHandler_t *handler)
{
  if (!state.headHandler)
  {
    state.headHandler = handler;
    handler->next = NULL;
  }
  else
  {
    handler->next = state.headHandler;
    state.headHandler = handler;
  }

  return 0;
}

int webserverWSRegister(WebSocketDataHandler_t *handler)
{
  if (!state.headWSHandler)
  {
    state.headWSHandler = handler;
    handler->next = NULL;
  }
  else
  {
    handler->next = state.headWSHandler;
    state.headWSHandler = handler;
  }

  return 0;
}

void webServerBroadcastData(uint8_t *data, int len)
{
  //portENTER_CRITICAL_SAFE(&buf_spinlock);
  struct mg_connection *c = NULL;
  for (c = mg_next(&state.mgr, NULL); c != NULL && (c->flags & MG_F_IS_WEBSOCKET); c = mg_next(&state.mgr, c))
    mg_send_websocket_frame(c, WEBSOCKET_OP_BINARY, data, len);
  //portEXIT_CRITICAL_SAFE(&buf_spinlock);
}

void webServerSendJson(struct mg_connection *c, cJSON *json)
{
  //portENTER_CRITICAL_SAFE(&buf_spinlock);
  char *str = cJSON_Print(json);
  int len = strlen(str);

  mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, str, len);
  cJSON_free(str);
  //portEXIT_CRITICAL_SAFE(&buf_spinlock);
}

void webServerBroadcastJson(cJSON *json)
{
  //portENTER_CRITICAL_SAFE(&buf_spinlock);

  char *str = cJSON_Print(json);
  int len = strlen(str);

  struct mg_connection *c = NULL;
  for (c = mg_next(&state.mgr, NULL); c != NULL && (c->flags & MG_F_IS_WEBSOCKET); c = mg_next(&state.mgr, c))
  {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, str, len);
  }

  cJSON_free(str);
  //portEXIT_CRITICAL_SAFE(&buf_spinlock);
}

data_type_write_decl(WebServerConfig_t)
{
  data_set_string(port);
}

data_type_read_decl(WebServerConfig_t)
{
  data_get_string(port);
}