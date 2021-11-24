import binascii
import struct
import bitstruct

SBUS_INPUT_MIN=192
SBUS_INPUT_MID=992
SBUS_INPUT_MAX=1792

SBUS_INPUT_RANGE = ((SBUS_INPUT_MAX - SBUS_INPUT_MIN) - 1)

BAUD = 115200
MARKER = 0x7E
MASK = 0x20
FRAME_SIZE = 0x19
FRAME_TYPE_CONTROL = 0x00

headerStruct = bitstruct.compile('u8u8u8')
channelsStruct = bitstruct.compile('u11u11u11u11u11u11u11u11u11u11u11u11u11u11u11u11u8u8')
footerStruct = bitstruct.compile('u8u8')

def crc(data):
  return 0x00

def cleanData(data):
  out = []
  for d in data:
    out.append(d)
    if( d == MARKER or d == 0x7D ):
      out.append(d ^ MASK)
  return out
      
def writeControls(channels, rssi, flags):
  h = list(headerStruct.pack(MARKER, FRAME_SIZE, FRAME_TYPE_CONTROL ))

  buf = channels
  buf.append(rssi)
  buf.append(flags)

  data = list(bytearray(channelsStruct.pack(*buf)))
  data.append(crc(data))
  data = cleanData(data)

  h = h + data
  h.append(MARKER)

#print(h)
  return h

  

