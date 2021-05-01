meta:
  id: dbf
  endian: le
seq:
  - id: p
    type: packet
    repeat: eos
types:
  packet:
    seq:
      - id: magic
        contents: [0x7C]
      - id: type
        type: u1
      - id: length
        type: u1
      - id: payload
        size: length
      - id: crc
        type: u1