
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display_controller.h"

static DisplayControllerConfig_t *config;
static DisplayControllerState_t state;

// DisplayBuffer_t* displayFrameBuffer()
// {
//     return &state.frameBuffer;
// }

// void displayClear()
// {
//     memset(state.frameBuffer.buffer, 0, sizeof(state.frameBuffer.buffer));
// }

// #define ON_PIXEL 0x01
// #define OFF_PIXEL 0x00

// void displayDraw(int x, int y, int pixel)
// {
//     uint16_t pos = ((uint16_t)x) + (y >> 3) * DISPLAY_WIDTH;

//     if(pixel == OFF_PIXEL)
//         state.frameBuffer.buffer[pos] &= ~(ON_PIXEL << (y%8));
//     else
//         state.frameBuffer.buffer[pos] |= ON_PIXEL << (y%8);
// }