
#include "telemetry.h"
#include "telemetry_native.h"
#include "system.h"
#include <string.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  TDataVar_t* ptr;
#if TELEMETRY_PRE_ALLOC
  TDataVar_t var;
#endif
} TableNode_t;

typedef struct
{
  TableNode_t* dataTable;
  uint32_t itemCount;

  TDataModPacket_t *valueMods;
  volatile uint32_t valueModCount;

  char* st_head;
  char* st_tail;
  

  uint32_t frameCount;
  uint64_t frameTime;

  bool sending;

  TelemetryConfig_t *cfg;
} TelemetryState_t;

static TelemetryState_t s;




DEF_STATIC_DATA_VAR(tdv_next_desc_send, 0,
                    "telemetry.next_desc_send",
                    "The next description meta data value to send",
                    u32, Tdm_RW | Tdm_realtime);

DEF_STATIC_DATA_VAR(tdv_telemetry_queue, 0,
                    "telemetry.queue",
                    "Number of valud mods waiting to be sent",
                    u32, Tdm_read | Tdm_realtime);

DEF_STATIC_DATA_VAR(tdv_telemetry_update_us, 0,
                    "telemetry.update.us",
                    "Amount of time, in microseconds, taken to send a sample frame",
                    u32, Tdm_read | Tdm_realtime);


DEF_STATIC_DATA_VAR(tdv_telemetry_packets_recv, 0,
                    "telemetry.packets.recv",
                    "Number of packets received",
                    u32, Tdm_read | Tdm_realtime);


DEF_STATIC_DATA_VAR(tdv_telemetry_packets_error_crc, 0,
                    "telemetry.packets.err.crc",
                    "Number of packet crc errors",
                    u32, Tdm_read | Tdm_realtime);



DEF_DATA_VAR(tdv_telemetry_sample_buffer_count, 64,
  "telemetry.sample.buffer.count",
  "Number of samples to store in sample buffer",
  u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_telemetry_val_count, 1024,
  "telemetry.values.max",
  "Maximum number of data vars that the system can store",
  u32, Tdm_RW | Tdm_config);


DEF_DATA_VAR(tdv_telemetry_str_table, 0,
  "telemetry.vars.strings.size",
  "Size, in bytes, of string table for received descriptions",
  u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_telemetry_auto_register, false,
  "telemetry.vars.auto_register",
  "Maximum number of data vars that the system can store",
  b8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
void telemetry_mod_send_complete(int sentCount)
{
  s.sending = false;
  //s.valueModCount -= sentCount;
}

// ---------------------------------------------------------------
int string_table_size()
{
  return s.st_tail - s.st_head;
}

// ---------------------------------------------------------------
char* string_table_reserve(uint32_t size)
{
  if(s.st_tail == NULL || s.st_head == NULL)
    return NULL;

  if( tdv_telemetry_str_table.v.u32 - string_table_size() < size+1)
    return NULL;

  char* start = s.st_tail;
  s.st_tail += size+1;

  return start;
}

// ---------------------------------------------------------------
void string_table_reset()
{
  s.st_head = NULL;
  s.st_tail = NULL;

  if(tdv_telemetry_str_table.v.u32 > 0)
  {
    s.st_head = s.st_tail = malloc(tdv_telemetry_str_table.v.u32);
  }
}

// ---------------------------------------------------------------
void telemetry_init()
{
  printf("init telemetry system\n");

  memset(&s, 0, sizeof(s));


  s.itemCount = 0;
  int tableMemSize = sizeof(TableNode_t) * tdv_telemetry_val_count.v.u32;
  s.dataTable = malloc(tableMemSize);
  memset(s.dataTable, 0, tableMemSize);

  s.valueModCount = 0;
  int modsMemSize = sizeof(TDataVar_t*) * tdv_telemetry_sample_buffer_count.v.u32 + 256;
  s.valueMods = malloc(modsMemSize);
  memset(s.valueMods, 0, modsMemSize);



  string_table_reset();

  s.frameCount = 0;
  s.frameTime = 0;

  tdv_telemetry_update_us.v.u32 = system_time_us();

  telemetry_register_var(&tdv_next_desc_send);
  telemetry_register_var(&tdv_telemetry_update_us);
  telemetry_register_var(&tdv_telemetry_queue);

  telemetry_native_init(telemetry_mod_send_complete);
}





// ---------------------------------------------------------------
uint32_t index_table_get(TDataVar_t **t, uint32_t tableSize, uint32_t id)
{
  uint32_t hash = (id % tableSize);
  hash = hash < 1 ? 1 : hash;

  for (uint32_t i = hash; i < tableSize; ++i)
  {
    if (t[i] == NULL || t[i]->id == id)
      return i;
  }

  return 0;
}

// ---------------------------------------------------------------
void value_to_string(TDataVar_t* v, char* buf, int bufsize)
{
  switch(v->meta.type)
  {
  case Tdt_u8:
  case Tdt_u16:
  case Tdt_u32:
    snprintf(buf, bufsize, "%s=%u", v->meta.name, v->v.u32);
    break;

  case Tdt_i8:
  case Tdt_i16:
  case Tdt_i32:
    snprintf(buf, bufsize, "%s=%d", v->meta.name, v->v.i32);
    break;

  case Tdt_c8:
    snprintf(buf, bufsize, "%s=%c", v->meta.name, v->v.c8);
    break;

  case Tdt_b8:
    snprintf(buf, bufsize, "%s=%u", v->meta.name, v->v.b8);
    break;

  case Tdt_f32:
    snprintf(buf, bufsize, "%s=%f", v->meta.name, v->v.f32);
    break;
      
  default:
      break;  
  }
}

// ---------------------------------------------------------------
TDataVar_t* set_value_from_string(const char* buf, int bufsize)
{
  char* nameEnd = strstr(buf, "=");
  if(nameEnd == NULL)
    return NULL;

  char name[64];
  int len = nameEnd - buf;
  assert(len < sizeof(name));
  memcpy(name, buf, len);
  name[len] = 0;

  TDataVar_t* var = telemetry_get_var_by_name(name);
  if(!var)
    return NULL;

  if(!(var->meta.modsAllowed & Tdm_write))
    return NULL;

  TValue_t v;
  switch(var->meta.type)
  {
  case Tdt_b8:
  case Tdt_u8:
  case Tdt_u16:
  case Tdt_u32:
    v.u32 = (uint32_t)strtoul(nameEnd+1, NULL, 10);
    break;

  case Tdt_i8:
  case Tdt_i16:
  case Tdt_i32:
    v.i32 = (int32_t)strtol(nameEnd+1, NULL, 10);
    break;

  case Tdt_c8:
    v.c8 = *(nameEnd+1);
    break;

  case Tdt_f32:
    v.f32 = atof(nameEnd+1);
    break;
      
  default:
      break;  
  }

  var->v = v;

  telemetry_sample_var(var);

  return var;
}

// ---------------------------------------------------------------
void value_table_remove(uint32_t id)
{
  if (id < 1)
    return;

  s.dataTable[id].ptr = NULL;
}

// ---------------------------------------------------------------
TDataVar_t* value_table_get(uint32_t id)
{
  return s.dataTable[id].ptr;
}

// ---------------------------------------------------------------
TDataVar_t* telemetry_get_var_by_name(const char* name)
{
  // just a brute force search
  for(int id = 1; id < tdv_telemetry_val_count.v.u32; ++id)
  {
    TDataVar_t* v = s.dataTable[id].ptr;
    if(v != NULL && strcmp(v->meta.name, name) == 0)
      return v;
  }
  return NULL;
}

// ---------------------------------------------------------------
int telemetry_vars_count()
{
  return s.itemCount;
}

// ---------------------------------------------------------------
bool telemetry_register_var(TDataVar_t *dataVar)
{
  assert(dataVar->id == 0);
  uint32_t index = ++s.itemCount;
  dataVar->id = index;
  s.dataTable[index].ptr = dataVar;

  return true;
}

// ---------------------------------------------------------------
bool telemetry_register_var_array(TDataVar_t *dataVar, int count)
{
  TDataVar_t *v = dataVar;
  for (int i = 0; i < count; ++i, ++v)
  {
    assert(v->id == 0);
    uint32_t index = ++s.itemCount;
    v->id = index;
    s.dataTable[index].ptr = v;
  }

  return true;
}

// ---------------------------------------------------------------
void telemetry_sample_var(TDataVar_t *dataVar)
{
  assert(dataVar->id != 0);
  if(telemetry_native_sending())
    return;
  
  // buffer full
  // TODO: some usable error here
  if (s.valueModCount >= tdv_telemetry_sample_buffer_count.v.u32)
    return;

  int id = s.valueModCount++;
  tdv_telemetry_queue.v.u32 = s.valueModCount;
  TDataValueMod_t *mod = &(s.valueMods + id)->payload;

  mod->mod = Tdm_write;
  mod->value.id = dataVar->id;
  mod->time = 0.0f;
  mod->value.type = dataVar->meta.type;
  mod->value.value = dataVar->v;
}

// ---------------------------------------------------------------
void telemetry_sample_var_at(TDataVar_t *dataVar, float now_us)
{
  if(telemetry_native_sending())
    return;

  // buffer full
  // TODO: some usable error here
  if (s.valueModCount >= s.cfg->valueModBufferCount)
    return;

  int id = s.valueModCount++;
  tdv_telemetry_queue.v.u32 = s.valueModCount;
  TDataValueMod_t *mod = &(s.valueMods + id)->payload;

  mod->mod = Tdm_write;
  mod->value.id = dataVar->id;
  mod->time = now_us;
  mod->value.type = dataVar->meta.type;
  mod->value.value = dataVar->v;
}

// ---------------------------------------------------------------
void telemetry_sample_var_array(TDataVar_t *dataVar, int count)
{
  if(telemetry_native_sending())
    return;
  
  TDataVar_t *v = dataVar;
  for (int i = 0; i < count; ++i, ++v)
    telemetry_sample_var(v);
}

// ---------------------------------------------------------------
void telemetry_set_var(uint32_t id, TValue_t value, bool force)
{
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return;

  if(!force && !(v->meta.modsAllowed & Tdm_write))
    return;

  v->v = value;

  telemetry_sample_var(v);
}

// ---------------------------------------------------------------
TDataVar_t* telemetry_get_var(uint32_t id)
{
  TDataVar_t *v = value_table_get(id);
  return v;
}

// ---------------------------------------------------------------
uint8_t telemetry_calc_crc(uint8_t *buf, uint8_t size)
{
  uint8_t crc = *buf;
  for (int i = 1; i < size; ++i)
    crc ^= buf[i];

  return crc;
}

// ---------------------------------------------------------------
bool telemetry_write_all_to_file(const char* filename, uint32_t modsFilter)
{
  FILE*f = fopen(filename, "w");
  if(!f)
    return false;

  char buf[256];
  for(int id = 1; id < tdv_telemetry_val_count.v.u32; ++id)
  {
    TDataVar_t* v = s.dataTable[id].ptr;
    if(v == NULL)
      continue;

    if(!(v->meta.modsAllowed & modsFilter))
      continue;

    value_to_string(v, buf, sizeof(buf));
    fprintf(f, "%s\r\n", buf);
  }

  fclose(f);

  return true;
}

// ---------------------------------------------------------------
uint8_t *write_bytes(uint8_t *buf, uint8_t *data, int len)
{
  memcpy(buf, data, len);
  return buf + len;
}

// ---------------------------------------------------------------
uint8_t *write_string(uint8_t *buf, char *str)
{
  int len = strlen(str);
  assert(len < 256);

  uint8_t slen = (uint8_t)len;
  buf[0] = slen;

  memcpy(buf + 1, str, slen);
  return buf + slen + 1;
}

// ---------------------------------------------------------------
uint8_t* read_string(uint8_t *buf, char** outstr)
{
  uint8_t slen = buf[0];

  char* str = string_table_reserve(slen);

  if(str)
  {
    memcpy(str, buf + 1, slen);
    *(str+slen) = 0;

    int len = strlen(str);
    assert(len == slen);
    *outstr = str;
  }
  else
  {
    *outstr = NULL;
  }
  
  return buf + slen + 1;
}

// ---------------------------------------------------------------
uint8_t* read_bytes(uint8_t *buf, uint8_t *data, int len)
{
  memcpy(data, buf, len);
  return buf + len;
}

// // ---------------------------------------------------------------
// uint32_t telemetry_write_data_value_string(uint8_t* buf, TDataValue_t* value)
// {
//   uint8_t* sp = write_bytes(buf, )
// }


// ---------------------------------------------------------------
uint32_t __time_critical_func(telemetry_write_data_frame)(TDataModPacket_t *packet)
{
  packet->header.marker = TELEMETRY_MARKER_BYTE;
  packet->header.type = PKT_DATA_MOD;
  packet->header.size = sizeof(packet->payload);

  uint8_t *payload = (uint8_t *)&packet->payload;

  packet->crc = telemetry_calc_crc(payload, packet->header.size);
  return sizeof(TDataModPacket_t);
}

// ---------------------------------------------------------------
uint32_t __time_critical_func(telemetry_write_desc_frame)(
    TDataDescFramePacket_t *packet,
    TDataVar_t *var)
{
  packet->header.marker = TELEMETRY_MARKER_BYTE;
  packet->header.type = PKT_DATA_DESC_FRAME;

  uint8_t *start = (uint8_t *)packet;
  uint8_t *payload = start + sizeof(PacketHeader_t);
  uint8_t *sp = payload;

  sp = write_bytes(sp, (uint8_t *)&var->id, sizeof(var->id));
  sp = write_bytes(sp, (uint8_t *)&var->meta.type, sizeof(var->meta.type));
  sp = write_bytes(sp, (uint8_t *)&var->meta.modsAllowed, sizeof(var->meta.modsAllowed));
  sp = write_bytes(sp, (uint8_t *)&var->v, sizeof(var->v));
  
  sp = write_string(sp, var->meta.name);
  sp = write_string(sp, var->meta.desc);
  packet->header.size = sp - payload;

  *sp++ = telemetry_calc_crc(payload, packet->header.size);
  return sp - start;
}




static float last_ts = 0.0f;

// ---------------------------------------------------------------
void __time_critical_func(telemetry_update)()
{
   int max = 64;
  telemetry_native_recv(max);
}

// ---------------------------------------------------------------
void __time_critical_func(telemetry_send)(uint64_t start, uint64_t now)
{
  if(telemetry_native_sending())
    return;
  // if (dma_channel_is_busy(s.dmaId))
  //   return;

  telemetry_sample_var(&tdv_telemetry_queue);
  telemetry_sample_var(&tdv_telemetry_update_us);
  telemetry_sample_var(&tdv_next_desc_send);


  tdv_telemetry_update_us.v.u32 = system_time_us();
  uint32_t frameId = s.frameCount++;
  uint64_t lastTime = s.frameTime;
  s.frameTime = now;

  float ts = (now - start) / 1.0e6;

  last_ts = ts;

  bool sendsampled = false;
 
  uint32_t sendLength = 0;
  for (int p = 0; p < s.valueModCount; ++p)
  {
    TDataModPacket_t *packet = &s.valueMods[p];

    if (tdv_next_desc_send.v.u32 == packet->payload.value.id)
      sendsampled = true;

    if (packet->payload.time == 0.0f)
      packet->payload.time = ts;

    sendLength += telemetry_write_data_frame(packet);
  }

  // add a desc packet to the end if we need to send one
  if (tdv_next_desc_send.v.u32 < s.itemCount)
  {
    TDataVar_t *var = value_table_get(tdv_next_desc_send.v.u32 + 1);
    if(var)
    {
      if(!sendsampled)
      {
        telemetry_sample_var(var);

        TDataModPacket_t *packet = (s.valueMods + (s.valueModCount-1));

        if (packet->payload.time == 0.0f)
          packet->payload.time = ts;

        sendLength += telemetry_write_data_frame(packet);
      }

      // sendLength++;
      uint8_t *descStart = ((uint8_t *)s.valueMods) + sendLength;

      sendLength += telemetry_write_desc_frame((TDataDescFramePacket_t *)descStart, var);
    }
    ++tdv_next_desc_send.v.u32;
  }
  else
  {
    tdv_next_desc_send.v.u32 = 0;
  }

  telemetry_native_send(sendLength, (uint8_t*)s.valueMods, s.valueModCount);

  s.valueModCount = 0;

  uint32_t delta = system_time_us() - tdv_telemetry_update_us.v.u32;
  tdv_telemetry_update_us.v.u32 = delta;
}




// ---------------------------------------------------------------s
// ---------------------------------------------------------------
typedef enum
{
  RECV_RESET = 0,
  RECV_MARKER,
  RECV_HEADER_TYPE,
  RECV_HEADER_SIZE,
  RECV_PAYLOAD
} RecvState_t;

static RecvState_t recv_state = RECV_RESET;
static PacketHeader_t header;

typedef union
{
  TDataValueMod_t dataMod;
  uint8_t data[256];

} Payloads_t;

//TODO: make a union
static Payloads_t payloads;

uint8_t *buffer;
uint8_t offset = 0;
uint8_t crc;

// ---------------------------------------------------------------
// Node Version
// void processDataFrame()
// {
//   if (payloads.dataMod.mod != Tdm_write)
//     return;
    
//   telemetry_set_var(
//     payloads.dataMod.value.id,
//     payloads.dataMod.value.value, 
//     false
//     );
// }

// ---------------------------------------------------------------
void processDataFrame()
{
  // if (payloads.dataMod.mod != Tdm_write)
  //   return;
    
  telemetry_set_var(
    payloads.dataMod.value.id,
    payloads.dataMod.value.value, 
    true
    );
}


// ---------------------------------------------------------------
void processDataDescFrame()
{
  if(!tdv_telemetry_auto_register.v.b8)
    return;

  TDataVar_t var;

  uint8_t* sp = payloads.data;
  uint8_t size = header.size;

  sp = read_bytes(sp, (uint8_t *)&var.id, sizeof(var.id));

  TDataVar_t* v = value_table_get(var.id);
  if( v && v->meta.name)
    return;


  sp = read_bytes(sp, (uint8_t *)&var.meta.type, sizeof(var.meta.type));
  sp = read_bytes(sp, (uint8_t *)&var.meta.modsAllowed, sizeof(var.meta.modsAllowed));
  sp = read_bytes(sp, (uint8_t *)&var.v, sizeof(var.v));

  char* name = NULL;
  char* desc = NULL;
  sp = read_string(sp, &name);
  sp = read_string(sp, &desc);

  var.meta.name = name;
  var.meta.desc = desc;

  int read = sp-payloads.data;
  assert(read == size); 

  #if TELEMETRY_PRE_ALLOC
  TableNode_t* node = s.dataTable+var.id;
  v = node->ptr = &node->var;
  #endif

  v->id = var.id;
  v->v = var.v;
  v->meta = var.meta;
}

// ---------------------------------------------------------------
void processPacket()
{
  switch (header.type)
  {
  case PKT_DATA_MOD:
    processDataFrame();
    break;

  case PKT_DATA_DESC_FRAME:
    processDataDescFrame();
    break;

  case PKT_BATCH_START:
    break;

  case PKT_BATCH_END:
    break;
  }
}

// ---------------------------------------------------------------
void telemetry_recv(uint8_t byte)
{
  switch (recv_state)
  {
  case RECV_RESET:
    memset(&header, 0, sizeof(header));
    recv_state = RECV_MARKER;
    crc = 0;
    offset = 0;
    telemetry_recv(byte);
    break;

  case RECV_MARKER:
    if (byte == TELEMETRY_MARKER_BYTE)
    {
      header.marker = byte;
    }
    else if (header.marker == TELEMETRY_MARKER_BYTE)
    {
      header.type = byte;
      recv_state = RECV_HEADER_SIZE;

      switch (header.type)
      {
      case PKT_DATA_MOD:
        buffer = (uint8_t *)&payloads.dataMod;
        break;

      case PKT_DATA_DESC_FRAME:
        buffer = (uint8_t *)&payloads.data;
        break;

      case PKT_NONE:
      default:
        recv_state = RECV_RESET;
        break;
      }
    }
    break;

  case RECV_HEADER_SIZE:
    header.size = byte;
    // todo: make sure header size and buffer size are exactly the same
    recv_state = RECV_PAYLOAD;
    offset = 0;
    break;

  case RECV_PAYLOAD:
    if (offset < header.size)
    {
      if (offset == 0)
        crc = byte; 
      else 
        crc ^= byte;

      buffer[offset++] = byte;
    }
    else if (offset == header.size)
    {
      // printf("%d:%d\n", crc, byte);
      if (crc == byte)
      {
        // valid
        ++tdv_telemetry_packets_recv.v.u32;
        processPacket();
      }
      else
      {
        ++tdv_telemetry_packets_error_crc.v.u32;
      }

      recv_state = RECV_RESET;
    }
    break;
  }
}
