#ifndef __osd_h_INCLUDED__
#define __osd_h_INCLUDED__

#include "vector.h"
#include "stdbool.h"

void osdDraw(int x, int y, int pixel);
void osdDrawString(int x, int y, char* str);
void osdDrawString2(int x, int y, char* str);

void osdInit();
void osdUpdate();


#endif