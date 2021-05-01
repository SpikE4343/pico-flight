#include "vector.h"
#include <assert.h>
#include <stdio.h>

void print_vector4f(Vector4f_t* v)
{
  printf("(%f, %f, %f, %f)", v->x, v->y, v->z, v->w);
}

void print_vector4u16(Vector4u16_t* v)
{
  printf("(%u, %u, %u, %u)", v->x, v->y, v->z, v->w);
}

void print_vector3f(Vector3f_t* v)
{
  printf("(%f, %f, %f)", v->axis[0], v->axis[1], v->axis[2]);
}

void print_vector3u16(Vector3u16_t* v)
{
  printf("(%u, %u, %u)", v->x, v->y, v->z);
}

void print_vector3i16(Vector3i16_t* v)
{
  printf("(%d, %d, %d)", v->x, v->y, v->z);
}

void print_vector3i32(Vector3i32_t* v)
{
  printf("(%d, %d, %d)", v->x, v->y, v->z);
}
