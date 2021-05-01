#ifndef __osd_h_INCLUDED__
#define __osd_h_INCLUDED__

#include "vector.h"
#include "stdbool.h"


typedef struct
{
  uint8_t sync;
  uint8_t out;
} OsdConfig_t;


void osdDraw(int x, int y, int pixel);
void osdDrawString(int x, int y, char* str);
void osdDrawString2(int x, int y, char* str);

void osdInit(OsdConfig_t *_config);
void osdUpdate();


#endif