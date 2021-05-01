#ifndef display_controller_INCLUDED
#define display_controller_INCLUDED

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

typedef struct 
{
    uint32_t            updateDelay;
} DisplayControllerConfig_t;


typedef struct 
{
    uint8_t header;
    uint8_t buffer[DISPLAY_WIDTH*DISPLAY_HEIGHT/sizeof(uint8_t)];
} DisplayBuffer_t;

typedef struct
{
    DisplayBuffer_t frameBuffer;
} DisplayControllerState_t;

void displayInit(DisplayControllerConfig_t* info);
void displayDraw(int x, int y, int pixel);
void displayDrawLine(int sx, int sy, int ex, int ey, int pixel);
void displayDrawTri(int x0, int y0, int x1, int y1, int x2, int y2, int pixel );
void displayUpdate();
void displayClear();
DisplayBuffer_t* displayFrameBuffer();
#endif