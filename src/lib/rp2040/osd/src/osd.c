
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "osd.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>


#include "osd.pio.h"
#include "pico/platform.h"

#include "font8x8_basic.h"
#include "data_vars.h"

DEF_DATA_VAR(tdv_osd_pin_sync, 14, 
  "osd.pin.sync",
  "Gpio pin for NTSC video sync bit",
  u8, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_osd_pin_out, 15, 
  "osd.pin.output",
  "Gpio pin for NTSC video signal ouput bit",
  u8, Tdm_RW | Tdm_config);



#define LINE_SIZE (635) //(640)
#define LINE_COUNT 263
#define FRAME_VSYNC_LINES 20
#define FRAME_SIZE (LINE_SIZE * LINE_COUNT)

// ;
// ; 10 Mhz clock - gives 10 cyles per microsecond so multiply us valus by ten
// ; side set should be second bit for tri-state
// ;
// ; Can use two pins with 470 - sync, 1000 - putput, resistors
// ; | sync | output |  level  |
// ; |  0   |   0    |  sync   |
// ; |  1   |   0    |  black/blanking |
// ; |  1   |   1    |  white  |
// ; |  0   |   1    |  grey |

#define NTSC_LINE_FRONT_PORCH 14     //  ;  1.4 us
#define NTSC_LINE_HSYNC 47           //  ;  4.7 us
#define NTSC_LINE_VISIBLE_REGION (515)//+5) //  ; 51.5 us +5 to get to 640
#define NTSC_LINE_BACK_PORCH 59      //  ;  5.9 us

#define NTSC_LINE_VISIBLE_REGION_START (NTSC_LINE_HSYNC+NTSC_LINE_BACK_PORCH)
#define NTSC_LINE_VISIBLE_REGION_END (NTSC_LINE_VISIBLE_REGION_START+NTSC_LINE_VISIBLE_REGION)

#define TVOUT_2BIT_SYNC 0x00 // 0V
#define TVOUT_2BIT_BLANKING 0x02 // - 240mv
#define TVOUT_2BIT_BLACK 0x02 // - 240mv
#define TVOUT_2BIT_WHITE 0x03 // 1V
#define TVOUT_2BIT_GREY 0x01 // 0.5V or so?


typedef struct
{
  uint32_t fbSize;
  uint8_t dmaId;
  uint8_t dmaMask;
  // uint8_t dmaControlId;
  // uint8_t dmaControlMask;
  uint32_t frameTransferCount;
  uint8_t* frameBuffer;
  uint8_t* font;
} OsdState_t;

static OsdState_t s;

// ---------------------------------------------------------------
static void __time_critical_func(dma_complete_handler)()
{
  if (dma_hw->ints0 & s.dmaMask)
  {
    dma_channel_set_read_addr(s.dmaId, s.frameBuffer, true);
    //dma_channel_set_trans_count(s.dmaId, s.frameTransferCount, true);
    // clear IRQ
    dma_hw->ints0 = s.dmaMask;

   
    
  }
}

// ---------------------------------------------------------------
static void dma_init(PIO pio, uint sm)
{
  s.dmaId = dma_claim_unused_channel(true);
  s.dmaMask = 1u << s.dmaId;

  s.frameTransferCount = s.fbSize;

  irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler, PICO_HIGHEST_IRQ_PRIORITY);

  dma_channel_config c = dma_channel_get_default_config(s.dmaId);
  channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
  channel_config_set_irq_quiet(&c, false);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  //channel_config_set_chain_to(&c, s.dmaControlId);
  //channel_config_set_irq_quiet(&c, true);
  //channel_config_set_ring(&channel_config, false, 4);
  dma_channel_configure(
      s.dmaId,
      &c,
      &pio->txf[sm],
      &s.frameBuffer,
      s.frameTransferCount,
      false);

  // c = dma_channel_get_default_config(s.dmaControlId);
  // channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  // channel_config_set_read_increment(&c, true);
  // channel_config_set_write_increment(&c, true);
  // channel_config_set_irq_quiet(&c, true);
  // //channel_config_set_ring(&c, false, 2); // 1 << 3 byte boundary on write ptr
  // channel_config_set_ring(&c, true, 3); // 1 << 3 byte boundary on write ptr

  // dma_channel_configure(
  //     s.dmaControlId,
  //     &c,
  //     &dma_channel_hw_addr(s.dmaId)->al3_transfer_count, // Initial write address
  //     (&s.frameTransferCount),                        // Initial read address
  //     2,                                         // Halt after each control block
  //     false                                      // Don't start yet
  // );

  
  dma_channel_set_irq0_enabled(s.dmaId, true);
}

// ---------------------------------------------------------------
uint8_t line_bit_pattern(int pixel)
{
  int counter = 0;

  counter += NTSC_LINE_HSYNC;
  if (pixel < counter)
    return TVOUT_2BIT_SYNC;

  counter += NTSC_LINE_BACK_PORCH;
  if (pixel < counter)
    //return pixel % 2 == 0 ? TVOUT_2BIT_WHITE : TVOUT_2BIT_BLACK;
    return TVOUT_2BIT_BLANKING;

  counter += NTSC_LINE_VISIBLE_REGION;
  if (pixel < counter)
    //return pixel % 2 == 0 ? ( pixel % 4 == 0 ? TVOUT_2BIT_GREY : TVOUT_2BIT_WHITE) : TVOUT_2BIT_BLACK;
    return TVOUT_2BIT_BLACK;

  counter += NTSC_LINE_FRONT_PORCH;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  return TVOUT_2BIT_SYNC;
}

// ---------------------------------------------------------------
uint8_t vsync_field2_bit_pattern(int pixel)
{
  int counter = 0;

  counter += NTSC_LINE_HSYNC;
  if (pixel < counter)
    return TVOUT_2BIT_SYNC;

  counter += NTSC_LINE_BACK_PORCH;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  counter += NTSC_LINE_VISIBLE_REGION;
  if (pixel < counter)
    return TVOUT_2BIT_BLACK;

  counter += NTSC_LINE_FRONT_PORCH;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  return TVOUT_2BIT_SYNC;
}

// ---------------------------------------------------------------
uint8_t vsync_field1_bit_pattern(int pixel)
{
  int counter = 0;

  counter += NTSC_LINE_HSYNC/2;
  if (pixel < counter)
    return TVOUT_2BIT_SYNC;

  counter += NTSC_LINE_BACK_PORCH/2;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  counter += NTSC_LINE_VISIBLE_REGION/2;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  counter += NTSC_LINE_FRONT_PORCH/2;
  if (pixel < counter)
    return TVOUT_2BIT_BLANKING;

  return TVOUT_2BIT_SYNC;
}

// ---------------------------------------------------------------
uint8_t vsync_half_norm_bit_pattern(int pixel)
{
  int counter = 0;

  counter += NTSC_LINE_HSYNC >> 3;
  if (pixel < counter)
    return TVOUT_2BIT_SYNC;

  return TVOUT_2BIT_BLANKING;
}

// ---------------------------------------------------------------
uint8_t vsync_half_inv_bit_pattern(int pixel)
{
  int counter = 0;

  counter += (LINE_SIZE - (NTSC_LINE_HSYNC >> 1)) >> 1;
  if (pixel < counter)
    return TVOUT_2BIT_SYNC;

  return TVOUT_2BIT_BLANKING;
}

// ---------------------------------------------------------------
void initLine(int l)
{
  s.frameBuffer[l] = line_bit_pattern(l % LINE_SIZE );

  // printf("l%u:%X ", l / LINE_SIZE , s.frameBuffer[l]);
}

// ---------------------------------------------------------------
void initVsyncLine(int l)
{

  int line = l / LINE_SIZE;
  int local = l % (LINE_SIZE/2);
  if( line < 3)
    s.frameBuffer[l] = vsync_half_norm_bit_pattern(local);
  else if( line < 6)
    s.frameBuffer[l] = vsync_half_inv_bit_pattern(local);
  else if( line < 9 || (line == 8 && (l%LINE_SIZE) < LINE_SIZE/2))
    s.frameBuffer[l] = vsync_half_norm_bit_pattern(local);
  else
    s.frameBuffer[l] = vsync_field2_bit_pattern(l % LINE_SIZE);
  // printf("v%u:%X ", l / LINE_SIZE , s.frameBuffer[l]);
  // printf("\n");
}

// ---------------------------------------------------------------
void __time_critical_func(osdDraw)(int x, int y, int pixel)
{
  if(x >NTSC_LINE_VISIBLE_REGION_END)
    x = NTSC_LINE_VISIBLE_REGION_END;
  else if( x < 0)
    x = 0;

  if(y < 0)
    y = 0;
  else if(y > (LINE_COUNT-1))
    y = LINE_COUNT-1;

  int index =  ((y+FRAME_VSYNC_LINES) * LINE_SIZE) + (x + NTSC_LINE_VISIBLE_REGION_START);
  //printf("=[%d, %u]",index, pixel);
  s.frameBuffer[index] = pixel ? TVOUT_2BIT_WHITE : TVOUT_2BIT_BLACK;
}

// ---------------------------------------------------------------
void __time_critical_func(osdDrawString)(int x, int y, char* str)
{
  for(int h=0; h < 8; ++h)
  {
    int sx = x;
    int sy = y+h;
    char* ss = str;
    
    while(*ss)
    {
      char glyph = font8x8_basic[*ss][h];
      for(int cx=0; cx < 8; ++cx)
      {
        int set = glyph & 0x01;
        //printf("[%d, %d, %u]",sx + cx, sy, set);
        //if()
          osdDraw(sx + cx, sy, set);

        glyph >>= 1;
      }
      //printf("\n");

      sx+=8;
      ++ss;
    }
    //printf("\n");
  }
}

static uint8_t single_glyph[8] = { 
  0x55, //  0 1 0 1 0 1 0 1    
  0x30, //  0 0 1 1 0 0 0 0
  0x03, //  0 0 0 0 0 0 1 1
  0x30, //     X X     X X
  0x03, //     X X X X X X
  0x30, //     X X     X X
  0x03, //     X X     X X
  0x00 //                 
};

// ---------------------------------------------------------------
void __time_critical_func(osdDrawString2)(int x, int y, char* str)
{
  char* ss = str;
  int sx = x;

  while(*ss)
  {
    for(int h=0; h < 8; ++h)
    {
      
      int sy = y+h;
      
      uint8_t glyph = font8x8_basic[*ss][h];
      for(int cx=0; cx < 8; ++cx)
      {
        // printf("%u", set);
        //if()s
          osdDraw(sx + cx, sy, glyph & 0x01);
          //printf("%d ", glyph & 0x80);
          glyph >>= 1;
      }
        //printf("\n");
    }

    sx+=8;
    ++ss;
  }
}

// ---------------------------------------------------------------
void osdInit()
{
  printf("osd init begin\n");

  telemetry_register(&tdv_osd_pin_sync);
  telemetry_register(&tdv_osd_pin_out);

  s.fbSize = LINE_COUNT * LINE_SIZE;
  s.frameBuffer = malloc(s.fbSize);

  memset(s.frameBuffer, 0, sizeof(s.fbSize));

  // s.font = malloc(sizeof(font8x8_basic));

  // for(int g=0; g < 128; ++g)
  // {
  //   s.font[g * 8 + 0] = font8x8_basic[g][0];
  //   s.font[g * 8 + 1] = font8x8_basic[g][1];
  //   s.font[g * 8 + 2] = font8x8_basic[g][2];
  //   s.font[g * 8 + 3] = font8x8_basic[g][3];

  //   s.font[g * 8 + 4] = font8x8_basic[g][4];
  //   s.font[g * 8 + 5] = font8x8_basic[g][5];
  //   s.font[g * 8 + 6] = font8x8_basic[g][6];
  //   s.font[g * 8 + 7] = font8x8_basic[g][7];
  // }

  for (int l = 0; l < FRAME_SIZE; ++l)
  {
    if ((l / LINE_SIZE) < FRAME_VSYNC_LINES) // probably 18 and not 20 because of the extra line timing?
      initVsyncLine(l);
    else
      initLine(l);

    // if(l%LINE_SIZE == 0)
    //   printf("\n");
  }

  PIO pio = pio0;
  int sm = 1;
  uint offset = pio_add_program(pio, &osd_program);

  dma_init(pio, sm);


  //gpio_set_pulls(info->sync, false, true);
  //gpio_set_pulls(info->out, false, true);

  
  osd_program_init(pio, sm, offset, tdv_osd_pin_sync.v.u8, 2, 10000000);


  gpio_set_slewfast(tdv_osd_pin_sync.v.u8, true);
  gpio_set_slewfast(tdv_osd_pin_out.v.u8, true);

  // gpio_set_drive_strength(info->out, GPIO_DRIVE_12MA);
  // gpio_set_drive_strength(info->sync, GPIO_DRIVE_12MA);

  //osdDrawString(50, 100, "A B C D E F G H I J K L M N O P Q R");
  //osdDrawString2(50, 110, "A B C D E F G H I J K L M N O P Q R");
  

  //dma_channel_hw_addr(s.dmaId)->al3_read_addr_trig = (uintptr_t)s.frameBuffer;
  dma_start_channel_mask(s.dmaMask);

  printf("osd init end\n");

}
