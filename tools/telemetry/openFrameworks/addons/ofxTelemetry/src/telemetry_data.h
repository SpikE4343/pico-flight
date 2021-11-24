#ifndef __telemetry_data_h_INCLUDED__
#define __telemetry_data_h_INCLUDED__

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __time_critical_func
#define __time_critical_func(x) x
#endif

#define TELEMETRY_MARKER_BYTE 0x7C

typedef enum {
    Tdt_u8=0,
    Tdt_i8,
    Tdt_b8,
    Tdt_c8,
    Tdt_i16,
    Tdt_u16,
    Tdt_i32,
    Tdt_u32,
    Tdt_f32,
    // Tdt_i64,
    // Tdt_u64
    // Tdt_f64
} TDataType_t;


typedef enum {
    Tdm_read= 1 << 0,
    Tdm_write= 1 << 1,
    Tdm_RW = Tdm_read | Tdm_write,
    Tdm_restart_requ= 1 << 2,
    Tdm_config= 1 << 3,
    Tdm_realtime= 1 << 4,
} TDataModType_t;

typedef struct {
  uint8_t marker;
  uint8_t type;
  uint8_t size;
} __packed PacketHeader_t;

typedef union {
  // double f64;
  // uint64_t u64;
  // int64_t i64;
  
  float f32;

  uint32_t u32;

  int32_t i32;

  uint16_t u16;
  // uint16_t u16_arr[sizeof(TValue_t)/sizeof(uint16_t)];

  int16_t i16;
  // int16_t i16_arr[sizeof(TValue_t)/sizeof(int16_t)];

  uint8_t u8;
  // uint8_t u8_arr[sizeof(TValue_t)/sizeof(uint8_t)];

  int8_t i8;
  // int8_t i8_arr[sizeof(TValue_t)/sizeof(int8_t)];

  char c8;
  // char c8_arr[sizeof(TValue_t)/sizeof(char)];

  bool b8;
  // bool b8_arr[sizeof(TValue_t)/sizeof(bool)];
  
} __packed TValue_t;

typedef struct {
  uint32_t id;
  uint8_t type;
  TValue_t value;
} __packed TDataValue_t;

typedef struct {
  uint8_t mod;
  float time;
  TDataValue_t value;
} __packed TDataValueMod_t;




typedef struct {
  uint8_t type;
  uint8_t modsAllowed;
  char* name; // string
  char* desc; // string
} __packed TDataValueDesc_t;

typedef struct {
  uint32_t id;
  TValue_t v;
  TDataValueDesc_t meta;
} TDataVar_t;

// descriptions of available values
typedef struct {
  PacketHeader_t header;
  TDataVar_t payload;
  uint8_t crc;
} __packed TDataDescFramePacket_t;


typedef struct {
  PacketHeader_t header;
  TDataValueMod_t payload;
  uint8_t crc;
} __packed TDataModPacket_t;


typedef enum {
  PKT_NONE = 0,
  PKT_DATA_DESC_FRAME=50,
  PKT_DATA_MOD=100,
} TPacketType_t;


#define DECL_EXTERN_DATA_VAR(var_name) extern TDataVar_t var_name
#define DECL_EXTERN_DV_ARRAY(var_name) extern TDataVar_t var_name[]

#define DEF_DATA_VAR(var_name, default_value, meta_name, meta_desc, meta_type, meta_mods) TDataVar_t var_name = { .id = 0, .meta = {.name = meta_name, .desc=meta_desc, .type=Tdt_##meta_type, .modsAllowed=meta_mods}, .v = {.meta_type = default_value} }
#define DEF_STATIC_DATA_VAR(var_name, default_value, meta_name, meta_desc, meta_type, meta_mods) static DEF_DATA_VAR(var_name, default_value, meta_name, meta_desc, meta_type, meta_mods)

#define BEGIN_DEF_DV_ARRAY(var_name) TDataVar_t var_name[] = { 
#define BEGIN_STATIC_DEF_DV_ARRAY(var_name) static BEGIN_DEF_DV_ARRAY(var_name)


#define DEF_DV_ARRAY_ITEM(meta_index, default_value, meta_name, meta_desc, meta_type, meta_mods) { .id = 0, .meta = {.name = meta_name "." #meta_index, .desc=meta_desc, .type=Tdt_##meta_type, .modsAllowed=meta_mods}, .v = {.meta_type = default_value} }
#define DEF_DV_ARRAY_ITEM_NAMED(default_value, meta_name, meta_desc, meta_type, meta_mods) { .id = 0, .meta = {.name = meta_name, .desc=meta_desc, .type=Tdt_##meta_type, .modsAllowed=meta_mods}, .v = {.meta_type = default_value} }

#define END_DEF_DV_ARRAY() }

#define f32v(var) var.v.f32
#define u32v(var) var.v.u32
#define i32v(var) var.v.i32
#define u16v(var) var.v.u16
#define i16v(var) var.v.i16
#define u8v(var) var.v.u8
#define i8v(var) var.v.i8
#define char8v(var) var.v.c8
#define bool8v(var) var.v.b8



#endif