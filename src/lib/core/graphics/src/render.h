#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "stdint.h"
#include "block.h"
#include "math_game.h"

#define MAX_BUFFER_SIZE 4


typedef struct  
{
  uint8_t r, g, b;
} Color_t;

typedef struct 
{
  int iVisibleObjects;
  float fFrameTime;
} Stats_t;

typedef struct
{
  int iWidth;
  int iHeight;
  int iVisibleObjects;
  int iFrameCount;

  Matrix_t viewMatrix;
  Matrix_t viewDebugMatrix;
  Matrix_t projMatrix;

  float fFarPlane;
  float fNearPlane;
  float fFOV;
  float fScreenRatio;

  Frustum viewFrustum;

  Vector4_t vSunPos;
  Vector4_t vSunColor;

  //RenderObjectBlock aRenderBlock;

  //char pTempBuffer[MAX_BUFFER_SIZE];
} RendererData_t;

int render_initialize(int iWidth, int iHeight);
int render_init_font();
int render_begin_frame();
int render_draw();
int render_end_frame();
int render_shutdown();

int render_draw_text( const char* pText, int x, int y );

//int render_clone_renderobject( int existing, Vector4 pos );
//int render_load_game_model( const char* pFileName );
// RendererData_t* render_data();
// #define s_renderer render_data()

//unsigned int render_load_render_shader ( const char *fileName );

int render_draw_object(int iObj, uint16_t* iCurrentShader, uint16_t* iCurrentMesh);

//int render_load_texture( const char* pFileName, int shader );
//bool render_load_tga( const char *fileName, char **buffer, int *width, int *height );

Matrix_t* render_view_matrix();


void render_apply_vertex_shader(Matrix_t* modelViewProjection, int index, Vector4_t* inVertex, Vector4_t* outVertex );
void render_draw_pixel( int x, int y, Color_t color);
void render_draw_triangle(Matrix_t* modelViewProjection, int tri, uint16_t* indicies, Vector4_t* verts );
void render_draw_mesh(const Matrix_t* modelViewProjection, int mesh );

#endif
