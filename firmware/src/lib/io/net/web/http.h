#ifndef __net_web_http_h_INCLUDED__
#define __net_web_http_h_INCLUDED__

// ---------------------------------------------------------------
typedef enum {
    Http_GET=1,
    Http_PUT,
    Http_POST,
    Http_DELETE,
} HttpMethod_t;

// ---------------------------------------------------------------
typedef enum {
    hps_reset=0,
    hps_fail,
    hps_method,
      hps_method_url,
      hps_method_version,
      hps_method_end_r,
      hps_method_end_n,
    hps_headers,
    hps_headers_check_end_r,
    hps_headers_check_end_n,
    hps_body,
    hps_complete,
} HttpParseState_t;


// ---------------------------------------------------------------
typedef enum {
    hvps_id,
    hvps_sep,
    hvps_value,
    hvps_complete,
    hvps_fail,
} HttpHeaderParseState_t;

// ---------------------------------------------------------------
typedef struct {
  char* data;
  uint8_t length;
} HttpStr_t;

// ---------------------------------------------------------------
typedef struct {
  HttpStr_t id;
  HttpStr_t value;
  HttpHeaderParseState_t state;
} HttpHeader_t;

// ---------------------------------------------------------------
#define HTTP_MAX_HEADER_COUNT 64

typedef struct {
  // Method 
  HttpMethod_t method;
  HttpStr_t method_p;

  HttpStr_t url;

  HttpStr_t version;

  // Headers
  HttpHeader_t headers[HTTP_MAX_HEADER_COUNT];
  uint8_t header_count;

  // Body
  HttpStr_t body;

  HttpParseState_t state;
} HttpRequest_t;

int http_read_request(HttpRequest_t* req, uint8_t* data, uint32_t length);
void http_read(HttpRequest_t* req, uint8_t* c);

#endif