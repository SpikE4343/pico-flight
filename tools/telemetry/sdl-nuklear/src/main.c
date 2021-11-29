/* nuklear - 1.32.0 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define TELEMETRY_PRE_ALLOC 1

#include "telemetry.h"
#include "telemetry_native.h"
#include "serial.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION

#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024


static telemetry_native_send_callback_t sendCallback;

bool telemetry_native_sending()
{
  return false;
}

// static void __time_critical_func(dma_complete_handler)()
// {
//   if (dma_hw->ints1 & s.dmaMask)
//   {
//     // clear IRQ
//     dma_hw->ints1 = s.dmaMask;
//     if(s.sendCallback)
//       s.sendCallback(s.sendingCount);

//     s.sendingCount = 0;
//   }
// }

// static void dma_init(uint8_t* packets)
// {
//   s.dmaId = dma_claim_unused_channel(true);
//   s.dmaMask = 1u << s.dmaId;

//   //irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler, PICO_DEFAULT_IRQ_PRIORITY+8);

//   dma_channel_config c = dma_channel_get_default_config(s.dmaId);
//   channel_config_set_dreq(&c, DREQ_UART0_TX);
//   channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
//   channel_config_set_read_increment(&c, true);
//   dma_channel_configure(
//       s.dmaId,
//       &c,
//       &uart_get_hw(uart0)->dr,
//       packets,
//       0,
//       false);

//   //dma_channel_set_irq0_enabled(s.dmaId, true);
// }

// static void dma_send(uint32_t sendLength, uint8_t* packets, int packetCount)
// {
//   assert(!dma_channel_is_busy(s.dmaId));

//   s.sendingCount = packetCount;

//   dma_channel_set_read_addr(s.dmaId, packets, false);
//   dma_channel_set_trans_count(s.dmaId, sendLength, true);
// }

void telemetry_native_init(telemetry_native_send_callback_t sendCB)
{
  sendCallback = sendCB;
  // s.sendCallback = sendCB;
  // dma_init(NULL);
}


void telemetry_native_send(int length, uint8_t* packets, int packetCount)
{
  serial_write(packets, length);
  if(sendCallback)
  {
    sendCallback(packetCount);
  }
  // dma_send(length, packets, packetCount);
}

static uint32_t recv_bytes =0;
void telemetry_native_recv(int max)
{
  while(serial_available() && max-- > 0)
  {
    uint8_t rd = 0;
    if( serial_read(&rd,1) == 1)
    {
      ++recv_bytes;
      telemetry_recv(rd);
    }
  }
}



/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int main(void)
{
    /* Platform */
    SDL_Window *win;
    SDL_GLContext glContext;
    int win_width, win_height;
    int running = 1;

    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &win_width, &win_height);

    /* OpenGL setup */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    ctx = nk_sdl_init(win);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
      struct nk_font_atlas *atlas;
      nk_sdl_font_stash_begin(&atlas);
      /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
      /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
      /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
      /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
      /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
      /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
      nk_sdl_font_stash_end();
      /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
      /*nk_style_set_font(ctx, &roboto->handle);*/
    }

    serial_init();

    tdv_telemetry_str_table.v.u32 = 64 * 2 * tdv_telemetry_val_count.v.u32;
    tdv_telemetry_auto_register.v.b8 = true;
    telemetry_init();

    static char port[64];
    static int port_len;

    int baud = 115200;

    static const float ratio[] = {120, 150};

    strcpy(port, "/dev/ttyACM0");
    port_len = strlen(port);

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            nk_sdl_handle_event(&evt);
        } nk_input_end(ctx);

        telemetry_update();
        /* GUI */
        if (nk_begin(ctx, "Serial", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {

            nk_layout_row_dynamic(ctx, 25, 2);

            nk_label(ctx, "Port:", NK_TEXT_LEFT);
            nk_edit_string(ctx, NK_EDIT_SIMPLE, port, &port_len, 64, nk_filter_default);
            
            nk_label(ctx, "Baud:", NK_TEXT_LEFT);
            nk_property_int(ctx, "", 0, &baud, 4000000, 0, 1);

            nk_layout_row_dynamic(ctx, 25, 2);
            if( serial_is_open())
            {
               if (nk_button_label(ctx, "Disconnect"))
               {
                 serial_close();
               }

               
            }
            else
            {
              if (nk_button_label(ctx, "Connect"))
              {
                serial_open(port, baud);
              }
            }
            
            nk_labelf(ctx, NK_TEXT_LEFT, "rx: %u", recv_bytes);

            nk_layout_row_dynamic(ctx, 25, 1);
            for(int id = 1; id < tdv_telemetry_val_count.v.u32; ++id)
            {
              TDataVar_t* v = telemetry_get_var(id);
              if(v)
              {
                nk_labelf(ctx, NK_TEXT_LEFT, "[%u] %s=%u", id, v->meta.name, v->v.u32);
              }
            }

            // enum {EASY, HARD};
            // static int op = EASY;
            // static int property = 20;

            // nk_layout_row_static(ctx, 30, 80, 1);
            // if (nk_button_label(ctx, "button"))
            //     printf("button pressed!\n");
            // nk_layout_row_dynamic(ctx, 30, 2);
            // if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            // if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            // nk_layout_row_dynamic(ctx, 22, 1);
            // nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            // nk_layout_row_dynamic(ctx, 20, 1);
            // nk_label(ctx, "background:", NK_TEXT_LEFT);
            // nk_layout_row_dynamic(ctx, 25, 1);
            // if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            //     nk_layout_row_dynamic(ctx, 120, 1);
            //     bg = nk_color_picker(ctx, bg, NK_RGBA);
            //     nk_layout_row_dynamic(ctx, 25, 1);
            //     bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            //     bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            //     bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
            //     bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
            //     nk_combo_end(ctx);
        }
        nk_end(ctx);

    
        /* ----------------------------------------- */

        /* Draw */
        SDL_GetWindowSize(win, &win_width, &win_height);
        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(win);
    }

cleanup:
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

