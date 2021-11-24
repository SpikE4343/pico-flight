#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "http.h"
#include <string.h>

// ---------------------------------------------------------------
// HTTP Request
// ---------------------------------------------------------------
// char http_request[] = "GET /index.html HTTP/1.0\r\n\
// Host: www.paulgriffiths.net\r\n\
// User-Agent: Lynx/2.8.1rel.2 libwww-FM/2.14\r\n\
// Accept-Encoding: gzip, compress\r\n\
// Accept-Language: en\r\n\
// \r\n";

// ---------------------------------------------------------------
// HTTP Response
// ---------------------------------------------------------------
// HTTP/1.0 200 OK\r\n
// Server: PGWebServ v0.1\r\n
// Content-Type: text/html\r\n
// \r\n

// ---------------------------------------------------------------
void http_read(HttpRequest_t* req, uint8_t* c)
{
  switch(req->state)
  {
    // ---------------------------------------------------------------
    case hps_reset:
      memset(req, 0, sizeof(HttpRequest_t));
      req->method_p.data = c;
      req->method_p.length = 0;
      req->state = hps_method;
      http_read(req, c);
      break;

    // ---------------------------------------------------------------
    case hps_method:
      if(*c == ' ')
      {
        if( strncmp(req->method_p.data, "GET", 3) == 0)
          req->method = Http_GET;
        else if( strncmp(req->method_p.data, "PUT", 3) == 0)
          req->method = Http_PUT;
        else if(strncmp(req->method_p.data, "POST", 4) == 0)
          req->method = Http_POST;
        else if(strncmp(req->method_p.data, "DELETE", 6) == 0)
          req->method = Http_DELETE;

        req->state = hps_method_url;
      }
      else
        ++req->method_p.length;
      break;

    // ---------------------------------------------------------------
    case hps_method_url:
      if(req->url.length == 0)
      {
        req->url.data = c;
        req->url.length = 1;
      }
      else if( *c != ' ')
      {
        ++req->url.length;
      }
      else
      {
        req->state = hps_method_version;
      }
      break;

    // ---------------------------------------------------------------
    case hps_method_version:
      if(req->version.length == 0)
      {
        req->version.data = c;
        req->version.length = 1;
      }
      else if( *c != '\r' )
      {
        ++req->version.length;
      }
      else 
      {
        req->state = hps_method_end_n;
      }
      break;
    // ---------------------------------------------------------------
    case hps_method_end_n:
      if(*c == '\n')
      {  
        req->state = hps_headers_check_end_r;
        req->header_count = 0;
      }

      break;

    // ---------------------------------------------------------------
    case hps_headers_check_end_r:
      if(*c == '\r')
      {  
        req->state = hps_headers_check_end_n;
      }
      else
      {
        req->state = hps_headers;
        http_read(req, c);
      }

      break;

       // ---------------------------------------------------------------
    case hps_headers_check_end_n:
      if(*c == '\n')
      {  
        req->state = hps_body;
        
      }

      break;

    // ---------------------------------------------------------------
    case hps_headers:
    {
      if( req->header_count == HTTP_MAX_HEADER_COUNT)
      {
        req->state = hps_fail;
        break;
      }

      HttpHeader_t* header = req->headers + req->header_count;
      switch(header->state)
      {
        // ---------------------------------------------------------------
        case hvps_id:
          if( header->id.length == 0)
          {
            header->id.data = c;
            header->id.length = 1;
          }
          else if( *c != ':' )
          {
            ++ header->id.length;
          }
          else 
          {
            header->state = hvps_sep;
          }
          break;

        // ---------------------------------------------------------------
        case hvps_sep:
          if(*c == ' ')
          {
            header->state = hvps_value;
          }
          break;

        // ---------------------------------------------------------------
        case hvps_value:
          if( header->value.length == 0)
          {
            header->value.data = c;
            header->value.length = 1;
          }
          else if( *c != '\r' )
          {
            ++ header->value.length;
          }
          else 
          {
            header->state = hvps_complete;
          }
          break;

        // ---------------------------------------------------------------
        case hvps_complete:
          if( *c == '\n' )
          {
            ++req->header_count;
            req->state = hps_headers_check_end_r;
          }
          break;
      }
      break;
    }

    // ---------------------------------------------------------------
    case hps_body:
      if(req->body.length == 0)
      {
        req->body.data = c;
        req->body.length = 1;
      }
      else
      {
        ++req->body.length;
      }
      break;
  }
}


int http_read_request(HttpRequest_t* req, uint8_t* data, uint32_t length)
{
  for(int i=0; i < length; ++i)
  {
    http_read(req, data++);
  }

  if(req->state == hps_fail)
    return 0;

  req->state = hps_complete;
  return 1;
}

#if TEST

#include "basic_test.h"

char http_request[] = "GET /index.html HTTP/1.0\r\n\
Host: www.example.com\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36\r\n\
Accept-Encoding: gzip, compress\r\n\
Accept-Language: en\r\n\
\r\n\
<body>\
</body>";

int test_parse_request()
{
  char* c = http_request;
  HttpRequest_t req;
  memset(&req, 0, sizeof(req));

  for(int i=0; i < sizeof(http_request)-1; ++i)
  {
    http_read(&req, c++);
  }

  req.state = hps_complete;

  BT_ASSERT(req.method == Http_GET);
  BT_ASSERT(req.header_count == 4);
  BT_ASSERT(req.body.length == 13);

  return 1;
}


BT_SETUP();

int main()
{
  BT_BEGIN();

    BT_ADD_TEST(test_parse_request);

  BT_END();
}

#endif