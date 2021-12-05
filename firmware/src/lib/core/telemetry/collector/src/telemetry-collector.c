
#include "telemetry_node.h"
#include "telemetry_native.h"
#include "system.h"
#include <string.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>

typedef struct
{
  TDataVar_t *dataTable;
  uint32_t itemCount;

  TDataModPacket_t *valueMods;
  volatile uint32_t valueModCount;
  

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


DEF_DATA_VAR(tdv_telemetry_sample_buffer_count, 64,
  "telemetry.sample.buffer.count",
  "Number of samples to store in sample buffer",
  u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_telemetry_val_count, 1024,
  "telemetry.values.max",
  "Maximum number of data vars that the system can store",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
void telemetry_mod_send_complete(int sentCount)
{
  s.sending = false;
  //s.valueModCount -= sentCount;
}

// ---------------------------------------------------------------
void telemetry_node_init()
{
  printf("init telemetry system\n");

  memset(&s, 0, sizeof(s));


  s.itemCount = 0;
  int tableMemSize = sizeof(TDataVar_t) * tdv_telemetry_val_count.v.u32;
  s.dataTable = malloc(tableMemSize);
  memset(s.dataTable, 0, tableMemSize);

  s.valueModCount = 0;
  int modsMemSize = sizeof(TDataVar_t*) * tdv_telemetry_sample_buffer_count.v.u32 + 256;
  s.valueMods = malloc(modsMemSize);
  memset(s.valueMods, 0, modsMemSize);

  s.frameCount = 0;
  s.frameTime = 0;

  tdv_telemetry_update_us.v.u32 = system_time_us();

  telemetry_node_register(&tdv_next_desc_send);
  telemetry_node_register(&tdv_telemetry_update_us);
  telemetry_node_register(&tdv_telemetry_queue);

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
void value_table_remove(uint32_t id)
{
  if (id < 1)
    return;

  s.dataTable[id] = NULL;
}

// ---------------------------------------------------------------
TDataVar_t *value_table_get(uint32_t id)
{
  return s.dataTable[id];
}

// ---------------------------------------------------------------
int telemetry_node_count()
{
  return s.itemCount;
}

// ---------------------------------------------------------------
bool telemetry_node_register(TDataVar_t *dataVar)
{
  assert(dataVar->id == 0);
  uint32_t index = ++s.itemCount;
  dataVar->id = index;
  s.dataTable[index] = dataVar;

  return true;
}

// ---------------------------------------------------------------
bool telemetry_node_register_array(TDataVar_t *dataVar, int count)
{
  TDataVar_t *v = dataVar;
  for (int i = 0; i < count; ++i, ++v)
  {
    assert(v->id == 0);
    uint32_t index = ++s.itemCount;
    v->id = index;
    s.dataTable[index] = v;
  }

  return true;
}

// ---------------------------------------------------------------
void telemetry_node_sample(TDataVar_t *dataVar)
{
  assert(dataVar->id != 0);
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
  mod->time = 0.0f;
  mod->value.type = dataVar->meta.type;
  mod->value.value = dataVar->v;
}

// ---------------------------------------------------------------
void telemetry_sample_at(TDataVar_t *dataVar, float now_us)
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
void telemetry_node_sample_array(TDataVar_t *dataVar, int count)
{
  if(telemetry_native_sending())
    return;
  
  TDataVar_t *v = dataVar;
  for (int i = 0; i < count; ++i, ++v)
    telemetry_node_sample(v);
}

// ---------------------------------------------------------------
void telemetry_node_set(uint32_t id, TValue_t value, bool force)
{
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return;

  if(!force && !(v->meta.modsAllowed & Tdm_write))
    return;

  v->v = value;

  telemetry_node_sample(v);
}

// ---------------------------------------------------------------
TValue_t telemetry_node_get(uint32_t id)
{
  TValue_t null = {0};
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return null;

  return v->v;
}

// ---------------------------------------------------------------
TDataVarDesc_t *telemetry_node_get_desc(uint32_t id)
{
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return NULL;

  return &v->meta;
}

// ---------------------------------------------------------------
uint8_t telemetry_node_calc_crc(uint8_t *buf, uint8_t size)
{
  uint8_t crc = *buf;
  for (int i = 1; i < size; ++i)
    crc ^= buf[i];

  return crc;
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
  sp = write_string(sp, var->meta.name);
  sp = write_string(sp, var->meta.desc);
  packet->header.size = sp - payload;

  *sp++ = telemetry_calc_crc(payload, packet->header.size);
  return sp - start;
}

static float last_ts = 0.0f;

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

  telemetry_node_sample(&tdv_telemetry_queue);
  telemetry_node_sample(&tdv_telemetry_update_us);
  telemetry_node_sample(&tdv_next_desc_send);


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
        telemetry_node_sample(var);

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

  telemetry_node_update();
}

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
  TDataVar_t dataDescFrame;
} Payloads_t;

//TODO: make a union
static Payloads_t payloads;

uint8_t *buffer;
uint8_t offset = 0;
uint8_t crc;

// ---------------------------------------------------------------
void processDataFrame()
{
  if (payloads.dataMod.mod != Tdm_write)
    return;
    
  telemetry_node_set(
    payloads.dataMod.value.id,
    payloads.dataMod.value.value, 
    false
    );
}

// ---------------------------------------------------------------
void processDataDescFrame()
{
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
void telemetry_node_recv(uint8_t byte)
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
        buffer = (uint8_t *)&payloads.dataDescFrame;
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
        processPacket();
      }

      recv_state = RECV_RESET;
    }
    break;
  }
}
