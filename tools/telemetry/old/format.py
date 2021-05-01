import serial

RESET=0
PATTERN=1
HEADER=2
PAYLOAD=3

  
state = RESET

pattern = [0x7C, 0x64, 0x00, 0x12]
matched = 0

def print_hex(d):
  print(format(d, '02X'), end=' ')

# with open('data-out.bin', "rb") as f:
with serial.Serial(port='/dev/ttyACM1', baudrate=115200, stopbits=1) as f:
  while True:    
    if state == RESET:
      matched = 0
      size = 0
      state = PATTERN
    elif state == PATTERN:
      d = f.read(1)[0]
      if d == pattern[matched]:
        matched += 1
      else:
        matched = 0
        print_hex(d)
        
      if matched == len(pattern):
        state = PAYLOAD
        size = d
        print("")
        [print_hex(v) for v in pattern]
        
    elif state == PAYLOAD:
      d = f.read(1)[0]
      print_hex(d)
      size -= 1
      if(size == 0):
        print("")
        state = RESET
        

