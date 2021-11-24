
#ifndef __vector_h_included__
#define __vector_h_included__

#include <stdint.h>
// #include <pico/stdlib.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

typedef union
{
  struct
  {
    float roll;
    float pitch;
    float yaw;
    float throttle;
  }__packed;

  struct
  {
    float x;
    float y;
    float z;
    float w;
  }__packed;
  float axis[4];
} Vector4f_t;

typedef union
{
  struct
  {
    uint16_t roll;
    uint16_t pitch;
    uint16_t yaw;
    uint16_t throttle;
  }__packed;

  struct
  {
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t w;
  }__packed;
  uint16_t axis[4];
}  Vector4u16_t;

typedef union
{
  struct
  {
    float roll;
    float pitch;
    float yaw;
  }__packed;

  struct
  {
    float x;
    float y;
    float z;
  }__packed;

  float axis[3];
} __packed Vector3f_t;

typedef union
{
  struct
  {
    uint16_t roll;
    uint16_t pitch;
    uint16_t yaw;
  } __packed;

  struct
  {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } __packed;

  uint16_t axis[3];
} Vector3u16_t;

typedef union
{
  struct
  {
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
  }__packed;

  struct
  {
    int16_t x;
    int16_t y;
    int16_t z;
  }__packed;

  int16_t axis[3];
} __packed Vector3i16_t;

typedef union
{
  struct
  {
    int32_t roll;
    int32_t pitch;
    int32_t yaw;
  } __packed;

  struct
  {
    int32_t x;
    int32_t y;
    int32_t z;
  } __packed;

  int32_t axis[3];
} Vector3i32_t;


void print_vector4f(Vector4f_t* v);
void print_vector4u16(Vector4u16_t* v);

void print_vector3f(Vector3f_t* v);
void print_vector3u16(Vector3u16_t* v);
void print_vector3i16(Vector3i16_t* v);

void print_vector3i32(Vector3i32_t* v);

#endif