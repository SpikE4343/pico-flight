from construct import *
from construct.core import Float32l, Int32ul, Int32sl, Int8ul, Int8sl, Int16ul, Int16sl
import time

from construct.lib.py3compat import int2byte

import packet
import io


class MsgTypes:
  DESC=50 
  MOD=100

MARKER_BYTE = 0x7c

class RebufferedBytesIO2(object):

    def __init__(self, substream, tailcutoff=None, readSize=lambda: 1024):
        self.substream = substream
        self.offset = 0
        self.rwbuffer = b""
        self.moved = 0
        self.tailcutoff = tailcutoff
        self.readSize = readSize

    def read(self, count=None):
        if count is None:
            raise ValueError("count must be integer, reading until EOF not supported")
        startsat = self.offset
        endsat = startsat + count
        if startsat < self.moved:
            raise IOError("could not read because tail was cut off")
        while self.moved + len(self.rwbuffer) < endsat:
            try:
                newdata = self.substream.read(self.readSize())
            except BlockingIOError:
                newdata = None
            if not newdata:
                time.sleep(0)
                continue
            self.rwbuffer += newdata
        data = self.rwbuffer[startsat-self.moved:endsat-self.moved]
        self.offset += count
        if self.tailcutoff is not None and self.moved < self.offset - self.tailcutoff:
            removed = self.offset - self.tailcutoff - self.moved
            self.moved += removed
            self.rwbuffer = self.rwbuffer[removed:]
        if len(data) < count:
            raise IOError("could not read enough bytes, something went wrong")
        return data

    def write(self, data):
        startsat = self.offset
        endsat = startsat + len(data)
        while self.moved + len(self.rwbuffer) < startsat:
            newdata = self.substream.read(128*1024)
            self.rwbuffer += newdata
            if not newdata:
                time.sleep(0)
        self.rwbuffer = self.rwbuffer[:startsat-self.moved] + data + self.rwbuffer[endsat-self.moved:]
        self.offset = endsat
        if self.tailcutoff is not None and self.moved < self.offset - self.tailcutoff:
            removed = self.offset - self.tailcutoff - self.moved
            self.moved += removed
            self.rwbuffer = self.rwbuffer[removed:]
        return len(data)

    def seek(self, at, whence=0):
        if whence == 0:
            self.offset = at
            return self.offset
        elif whence == 1:
            self.offset += at
            return self.offset
        else:
            raise ValueError("this class seeks only with whence: 0 and 1 (excluded 2)")

    def seekable(self):
        return True

    def tell(self):
        return self.offset

    def tellable(self):
        return True

    def cachedfrom(self):
        return self.moved

    def cachedto(self):
        return self.moved + len(self.rwbuffer)


class Rebuffered2(Subconstruct):
    r"""
    Caches bytes from underlying stream, so it becomes seekable and tellable, and also becomes blocking on reading. Useful for processing non-file streams like pipes, sockets, etc.

    .. warning:: Experimental implementation. May not be mature enough.

    :param subcon: Construct instance, subcon which will operate on the buffered stream
    :param tailcutoff: optional, integer, amount of bytes kept in buffer, by default buffers everything

    Can also raise arbitrary exceptions in its implementation.

    Example::

        Rebuffered(..., tailcutoff=1024).parse_stream(nonseekable_stream)
    """

    def __init__(self, subcon, tailcutoff=None, readSize=lambda: 1024):
        super(Rebuffered2, self).__init__(subcon)
        self.stream2 = RebufferedBytesIO2(None, tailcutoff=tailcutoff, readSize=readSize)

    def _parse(self, stream, context, path):
        self.stream2.substream = stream
        return self.subcon._parsereport(self.stream2, context, path)

    def _build(self, obj, stream, context, path):
        self.stream2.substream = stream
        return self.subcon._build(obj, self.stream2, context, path)


def crcXor(data):
  crc = data[0]
  l = len(data)
  for i in range(1,l):
    crc = crc ^ data[i]
  return crc
    
packet_type = Enum(Int8ul, 
    DESC=50, 
    MOD=100)

header = Struct(
  'marker' / Const(b"\x7C"), StopIf(this.marker != b"\x7C"),
  'type' / packet_type,
  'size' / Int8ul,
)



data_type = Enum(Byte, 
  Tdt_u8=0,
  Tdt_i8=1,
  Tdt_bool=2,
  Tdt_char=3,
  Tdt_i16=4,
  Tdt_u16=5,
  Tdt_i32=6,
  Tdt_u32=7,
  Tdt_f32=8)


mod_type = FlagsEnum(Byte,
  Tdm_read = 0x01,
  Tdm_write= 0x02,
  Tdm_RW = 0x03,
  Tdm_restart_requ= 0x04,
  Tdm_config=0x08)


data_value = Struct(
  "id" / Int32ul,
  "type" / data_type,
  "data" / Switch(this.type, {
    "Tdt_u8" : Aligned(4, Int8ul),
    "Tdt_i8" : Aligned(4, Int8sl),
    "Tdt_bool" : Aligned(4, Int8ul),
    "Tdt_char" : Aligned(4, Int8ul),
    "Tdt_u16": Aligned(4, Int16ul),
    "Tdt_i16": Aligned(4, Int16sl),
    "Tdt_u32": Int32ul,
    "Tdt_i32": Int32sl,
    "Tdt_f32" : Float32l
  }, Int32ul)
)

data_mod = Struct(
  "type" / mod_type,
  "time" / Float32l,
  "value" / data_value
)

data_frame = Struct(
  "id" / Int32ul,
  "mod" / data_mod
)

data_meta = Struct(
  "type" / Int8ul,
  "modsAllowed" / Int8ul,
  "name" / PascalString(Int8ul, "ascii"),
  "desc" / PascalString(Int8ul, "ascii"),
)

data_var = Struct(
  "id" / Int32ul,
  "meta" / data_meta,
)

def dataPayload(frame):
  return Struct(
    "payload" / RawCopy(frame),
    "crc" / Checksum(Int8ul,
                lambda data: crcXor(data),
                this.payload.data)
  )

data_frame_packet = Rebuffered2(Struct(
  "header" / header,
  "data" / Switch(this.header.type, {
    "MOD": dataPayload(data_frame),
    "DESC": dataPayload(data_var)
  })
), readSize=lambda: 64, tailcutoff=1024)

mod = {
  'header': { 
    'marker': 0x7C, 
    'type': 'MOD'
    },
  'data': {
    'payload': { 
      'value':{
        'id':0,
        'mod': {
          'type' : 0x02,
          'time' : 100,
          'value' : {
            'id': 19293,
            'type': 'Tdt_u32',
            'data': 1000 
          }
        }      
      }
    }
  }
}

class Receiver(packet.Receiver):
  RESET = 0
  MARKER = 1
  HEADER_TYPE = 2
  HEADER_SIZE = 3
  PAYLOAD = 4
  PROCESS_PAYLOAD = 5
  
  def __init__(self, handlers:dict) -> None:
      self.state = Receiver.RESET
      self.marker = 0
      self.type = 0
      self.size = 0
      self.payload = bytearray()
      self.crc = 0
      self.handlers = handlers
    
  def next(self, byte):
    # print(f"[{self.state} {self.type} {self.size} {self.crc}] -> {hex(byte)}")
    if self.state == Receiver.RESET:
      self.state = Receiver.MARKER
      self.payload.clear()
      self.crc = 0
      self.type = 0
      self.size = 0 
      self.marker = 0
      self.next(byte)
    
    elif self.state == Receiver.MARKER:
      if byte == MARKER_BYTE:
        self.marker = byte
      elif self.marker == MARKER_BYTE:
        self.type = byte
        self.state = Receiver.HEADER_SIZE
        
    elif self.state == Receiver.HEADER_SIZE:
      self.size = byte
      self.state = Receiver.PAYLOAD
    
    elif self.state == Receiver.PAYLOAD:
      self.payload.append(byte)
      read = len(self.payload)
      
      if read == 1:
        self.crc = byte
      elif read <= self.size:
        self.crc ^= byte
      elif self.crc == byte:
        self.state = Receiver.PROCESS_PAYLOAD
        self.processPayload(self.payload)
        self.state = Receiver.RESET
      else:
        print('CRC Failure: calculated:', self.crc, 'received:', byte )
        self.state = Receiver.RESET
    elif self.state == Receiver.PROCESS_PAYLOAD:
      print('Error incoming data:', byte, 'while processing previous payload:', self.type, self.size)
    

   
class Sender(packet.Sender):
  def __init__(self, handlers) -> None:
      super().__init__(handlers)
      
  def serialize(self, writer, type, payload):
    size = 0
    for b in payload:
      if size == 0:
        crc = b
      else:
        crc ^= b
        
      size += 1
    
    d = bytearray(
      [MARKER_BYTE,
       type,
       size]
    )
    
    d.extend(payload)
    d.append(crc)
      
    writer.write(d)
    
    

