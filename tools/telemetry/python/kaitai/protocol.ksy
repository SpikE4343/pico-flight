meta:
  id: telemetry
  file-extension: .bin
  endian: le
seq:
  - id: packet
    type: packet
    repeat: eos
types:
  packet:
    seq:
      - id: marker
        contents: [0x7C]
      - id: type
        type: u1
      - id: len
        type: u1
      - id: payload
        size: len
        type:
          switch-on: type
          cases:
            50: data_desc_frame
            100: data_frame
  
      - id: crc
        type: u1
  data_frame:
    seq:
      - id: frame_id
        type: u4
      - id: mod
        type: mod_v
        
  mod_v:
    seq:
      - id: type
        type: u1
      - id: time
        type: f4
      - id: value
        type: data_value
        
  data_value:
    seq: 
      - id: id
        type: u4
      - id: type
        type: u1
      - id: data
        size: 4
        type:
          switch-on: type
          cases:
            0: u1
            1: s1
            2: b1
            3: s1
            4: u2
            5: s2
            6: s4
            7: u4
            8: f4
        
  data_desc_frame:
    seq:
      - id: id
        type: u4
      - id: meta
        type: value_meta
        
        
  value_meta:
    seq:
      - id: type
        type: u1
      - id: mods_allowed
        type: u1
      - id: name
        type: prefix_str
      - id: desc
        type: prefix_str
        
  prefix_str:
    seq:
      - id: len
        type: u1
      - id: str
        type: str
        size: len
        encoding: ascii
        
