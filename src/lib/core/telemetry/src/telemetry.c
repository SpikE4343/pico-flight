
#include "telemetry.h"
#include <string.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>

#include "hardware/timer.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

typedef struct
{
  TDataVar_t **dataTable;
  uint32_t itemCount;

  TDataFramePacket_t *valueMods;
  volatile uint32_t valueModCount;
  volatile uint32_t sendingCount;

  uint32_t frameCount;
  uint64_t frameTime;

  uint8_t dmaId;
  uint8_t dmaMask;

  TelemetryConfig_t *cfg;
} TelemetryState_t;

static TelemetryState_t s;

#define MARKER_BYTE 0x7C

static uint64_t get_time(void)
{
  // Reading low latches the high value
  uint32_t lo = timer_hw->timelr;
  uint32_t hi = timer_hw->timehr;
  return ((uint64_t)hi << 32u) | lo;
}

DEF_STATIC_DATA_VAR(tdv_next_desc_send, 0,
                    "telemetry.next_desc_send",
                    "The next description meta data value to send",
                    Tdt_u32, Tdm_RW);

DEF_STATIC_DATA_VAR(tdv_telemetry_queue, 0,
                    "telemetry.queue",
                    "Number of valud mods waiting to be sent",
                    Tdt_u32, Tdm_read);

DEF_STATIC_DATA_VAR(tdv_telemetry_update_us, 0,
                    "telemetry.update.us",
                    "Amount of time, in microseconds, taken to send a sample frame",
                    Tdt_u32, Tdm_read);


DEF_DATA_VAR(tdv_telemetry_sample_buffer_count, 64,
  "telemetry.sample.buffer.count",
  "Number of samples to store in sample buffer",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_telemetry_val_count, 1024,
  "telemetry.values.max",
  "Maximum number of data vars that the system can store",
  Tdt_u32, Tdm_RW | Tdm_config);

static void __time_critical_func(dma_complete_handler)()
{
  if (dma_hw->ints1 & s.dmaMask)
  {
    // clear IRQ
    dma_hw->ints1 = s.dmaMask;
    s.valueModCount = s.valueModCount - s.sendingCount;
    s.sendingCount = 0;
  }
}

static void dma_init()
{
  s.dmaId = dma_claim_unused_channel(true);
  s.dmaMask = 1u << s.dmaId;

  //irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler, PICO_DEFAULT_IRQ_PRIORITY+8);

  dma_channel_config c = dma_channel_get_default_config(s.dmaId);
  channel_config_set_dreq(&c, DREQ_UART0_TX);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  dma_channel_configure(
      s.dmaId,
      &c,
      &uart_get_hw(uart0)->dr,
      s.valueMods,
      0,
      false);

  //dma_channel_set_irq0_enabled(s.dmaId, true);
}

static void dma_send(uint32_t sendLength)
{
  if (dma_channel_is_busy(s.dmaId))
    return;

  s.sendingCount = s.valueModCount;

  dma_channel_set_read_addr(s.dmaId, s.valueMods, false);
  dma_channel_set_trans_count(s.dmaId, sendLength, true);
}

void telemetryInit()
{
  memset(&s, 0, sizeof(s));


  s.itemCount = 0;
  int tableMemSize = sizeof(TDataVar_t*) * tdv_telemetry_val_count.v.u32;
  s.dataTable = malloc(tableMemSize);
  memset(s.dataTable, 0, tableMemSize);

  s.valueModCount = 0;
  int modsMemSize = sizeof(TDataVar_t*) * tdv_telemetry_sample_buffer_count.v.u32;
  s.valueMods = malloc(modsMemSize);
  memset(s.valueMods, 0, modsMemSize);

  s.frameCount = 0;
  s.frameTime = 0;

  telemetry_register(&tdv_telemetry_queue);
  telemetry_register(&tdv_telemetry_update_us);

  tdv_telemetry_update_us.v.u32 = timer_hw->timelr;

  dma_init();
}

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

//================================
// Value table

void value_table_remove(uint32_t id)
{
  if (id < 1)
    return;

  s.dataTable[id] = NULL;
}

TDataVar_t *value_table_get(uint32_t id)
{
  return s.dataTable[id];
}

bool telemetry_register(TDataVar_t *dataVar)
{
  assert(dataVar->id == 0);
  uint32_t index = ++s.itemCount;
  dataVar->id = index;
  s.dataTable[index] = dataVar;

  return true;
}

bool telemetry_register_array(TDataVar_t *dataVar, int count)
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

void telemetry_sample(TDataVar_t *dataVar)
{
  // buffer full
  // TODO: some usable error here
  if (s.valueModCount >= s.cfg->valueModBufferCount)
    return;

  if (dma_channel_is_busy(s.dmaId))
    return;

  int id = s.valueModCount++;
  tdv_telemetry_queue.v.u32 = s.valueModCount;
  TDataValueMod_t *mod = &(s.valueMods + id)->payload.value;

  mod->mod = Tdm_write;
  mod->value.id = dataVar->id;
  mod->time = 0.0f;
  mod->value.type = dataVar->meta.type;
  mod->value.value = dataVar->v;
}

void telemetry_sample_at(TDataVar_t *dataVar, float now_us)
{
  // buffer full
  // TODO: some usable error here
  if (s.valueModCount >= s.cfg->valueModBufferCount)
    return;

  int id = s.valueModCount++;
  tdv_telemetry_queue.v.u32 = s.valueModCount;
  TDataValueMod_t *mod = &(s.valueMods + id)->payload.value;

  mod->mod = Tdm_write;
  mod->value.id = dataVar->id;
  mod->time = now_us;
  mod->value.type = dataVar->meta.type;
  mod->value.value = dataVar->v;
}

void telemetry_sample_array(TDataVar_t *dataVar, int count)
{
  TDataVar_t *v = dataVar;
  for (int i = 0; i < count; ++i, ++v)
    telemetry_sample(v);
}

void telemetry_set(uint32_t id, TValue_t value)
{
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return;

  v->v = value;
}

TValue_t telemetry_get(uint32_t id)
{
  TValue_t null = {0};
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return null;

  return v->v;
}

TDataValueDesc_t *telemetry_get_desc(uint32_t id)
{
  TDataVar_t *v = value_table_get(id);
  if (v == NULL)
    return NULL;

  return &v->meta;
}

uint8_t telemetry_calc_crc(uint8_t *buf, uint8_t size)
{
  uint8_t crc = *buf;
  for (int i = 1; i < size; ++i)
    crc ^= buf[i];

  return crc;
}

uint8_t *write_bytes(uint8_t *buf, uint8_t *data, int len)
{
  memcpy(buf, data, len);
  return buf + len;
}

uint8_t *write_string(uint8_t *buf, char *str)
{
  int len = strlen(str);
  assert(len < 256);

  uint8_t slen = (uint8_t)len;
  buf[0] = slen;

  memcpy(buf + 1, str, slen);
  return buf + slen + 1;
}

uint32_t __time_critical_func(telemetry_write_data_frame)(TDataFramePacket_t *packet)
{
  packet->header.marker = MARKER_BYTE;
  packet->header.type = PKT_DATA_MOD;
  packet->header.size = sizeof(packet->payload);

  uint8_t *payload = (uint8_t *)&packet->payload;

  packet->crc = telemetry_calc_crc(payload, packet->header.size);
  return sizeof(TDataFramePacket_t);
}

uint32_t __time_critical_func(telemetry_write_desc_frame)(
    TDataDescFramePacket_t *packet,
    TDataVar_t *var)
{
  packet->header.marker = MARKER_BYTE;
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

void __time_critical_func(telemetry_send)(uint64_t start, uint64_t now)
{
  if (dma_channel_is_busy(s.dmaId))
    return;

  telemetry_sample(&tdv_telemetry_queue);
  telemetry_sample(&tdv_telemetry_update_us);

  tdv_telemetry_update_us.v.u32 = timer_hw->timelr;

  uint32_t frameId = s.frameCount++;
  uint64_t lastTime = s.frameTime;
  s.frameTime = now;

  float ts = (now - start) / 1.0e6;

  last_ts = ts;

  uint32_t sendLength = 0;
  for (int p = 0; p < s.valueModCount; ++p)
  {
    TDataFramePacket_t *packet = (s.valueMods + p);
    packet->payload.frameId = frameId;
    if (packet->payload.value.time == 0.0f)
      packet->payload.value.time = ts;

    sendLength += telemetry_write_data_frame(packet);
  }

  // printf("--- s ---\n");
  // add a desc packet to the end if we need to send one
  if (tdv_next_desc_send.v.u32 < s.itemCount)
  {
    uint8_t *descStart = ((uint8_t *)s.valueMods) + sendLength;

    TDataVar_t *var = value_table_get(tdv_next_desc_send.v.u32 + 1);
    // printf("--- %s --- 1\n", var->meta.name);

    sendLength += telemetry_write_desc_frame((TDataDescFramePacket_t *)descStart, var);
    ++tdv_next_desc_send.v.u32;

    // printf("--- %s --- 2\n", var->meta.name);
  }
  else
  {
    tdv_next_desc_send.v.u32 = 0;
  }

  dma_send(sendLength);

  s.valueModCount = 0;

  uint32_t delta = timer_hw->timelr - tdv_telemetry_update_us.v.u32;
  tdv_telemetry_update_us.v.u32 = delta;
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
  TDataFrame_t dataFrame;
  TDataVar_t dataDescFrame;
} Payloads_t;

//TODO: make a union
static Payloads_t payloads;

uint8_t *buffer;
uint8_t offset = 0;
uint8_t crc;

void processDataFrame()
{
  if (payloads.dataFrame.value.mod == Tdm_write)
  {
    telemetry_set(
        payloads.dataFrame.value.value.id,
        payloads.dataFrame.value.value.value);
  }
}

void processDataDescFrame()
{
}

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
  }
}

void telemetry_recv(uint8_t byte)
{
  switch (recv_state)
  {
  case RECV_RESET:
    memset(&header, 0, sizeof(header));
    recv_state = RECV_MARKER;
    crc = 0;
    offset = 0;
    break;

  case RECV_MARKER:
    if (byte == MARKER_BYTE)
      header.marker = byte;
    else if (header.marker == MARKER_BYTE)
      recv_state = RECV_HEADER_TYPE;

    break;

  case RECV_HEADER_TYPE:
    header.type = byte;
    recv_state = RECV_HEADER_SIZE;

    switch (header.type)
    {
    case PKT_DATA_MOD:
      buffer = (uint8_t *)&payloads.dataFrame;
      break;

    case PKT_DATA_DESC_FRAME:
      buffer = (uint8_t *)&payloads.dataDescFrame;
      break;

    case PKT_NONE:
    default:
      recv_state = RECV_RESET;
      break;
    }

    break;

  case RECV_HEADER_SIZE:
    header.size = byte;
    recv_state = RECV_PAYLOAD;
    offset = 0;
    break;

  case RECV_PAYLOAD:
    if (offset == 0)
      crc = byte;

    if (offset < header.size)
    {
      buffer[offset++] = byte;
      crc ^= byte;
    }
    else if (offset == header.size)
    {
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
