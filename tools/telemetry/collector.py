from collections import defaultdict
from tabulate import tabulate

import serial
import time

from lognplot.client import LognplotTcpClient

import telemetry as telem

import threading
import queue
import re
import packet

from cmd import Cmd

setre = re.compile("(.*?)\s*=\s*(\d+|true|false)")
cmd_quit = "quit"
commands = queue.Queue()
class SerialReader(packet.Reader):
  def __init__(self, serial) -> None:
      super().__init__()
      self.serial = serial
      
  def available(self):
    if not self.serial.isOpen():
      return
    
    data = self.serial.in_waiting
    if data <= 0:
      return None
    
    read = self.serial.read(data)
    for d in read:
      yield d
      
class SerialWriter(packet.Writer):
  def __init__(self, serial) -> None:
    self.serial = serial
    
  def write(self, data):
    print('w:', data)
    self.serial.write(data)  


class DataReader:
    def __init__(self):

        self.client = LognplotTcpClient()
        self.connected = False
        self.lnpConnect()

        self.ser = serial.Serial(baudrate=115200, stopbits=1)

        self.samples = {}
        self.sampleCounts = defaultdict(int)
        self.sampleTime = defaultdict(float)
        self.descriptions = {}
        self.sampleNames = {}

        self.dumpOnly = False
        # self.dumpFile = open("data-out.bin", "wb")
        # self.file = open("data-in.bin", "rb")

        self.updates = 0

        self.readpos = 0
        self.crcErrors = 0

        # telem.data_frame_packet.stream2.readSize = self.inputWaitingData

        # self.data_frame_packet = telem.data_frame_packet.compile()
        
        self.packetizer = packet.Packetizer(
          receiver = telem.Receiver(
            handlers = {
              telem.MsgTypes.DESC: self.recvDescPayload, 
              telem.MsgTypes.MOD: self.recvModPayload
            }),
          sender = telem.Sender(
            handlers = {
              telem.MsgTypes.DESC: telem.data_var, 
              telem.MsgTypes.MOD: telem.data_frame
            }),
          reader = SerialReader(self.ser),
          writer = SerialWriter(self.ser)
          )

    def recvDescPayload(self, payload):
      desc = telem.data_var.parse(payload)
      self.descriptions[desc.id] = desc.meta
      self.sampleNames[desc.meta.name] = desc.id
    
    def recvModPayload(self, payload):
      msg = telem.data_frame.parse(payload)
      id = msg.mod.value.id
      value = msg.mod.value
      st = msg.mod.time
      fst = float(st)

      self.samples[id] = value
      self.sampleCounts[id] += 1

      if self.sampleTime[id] > fst:
          print(
              "Time rolled back:",
              id,
              fst - self.sampleTime[id],
              fst,
              self.sampleTime[id],
              id,
          )
      else:
          meta = self.descriptions.get(value.id)
          if meta is None:
              name = value.id
          else:
              name = meta.name

          self.send_sample(
              str(name), timestamp=fst, value=float(value.data)
          )

      self.sampleTime[id] = fst
      self.msgCount += 1
    
    

    def lnpConnect(self):
        try:
            self.client.connect()
            self.connected = True
        except:
            pass
          
    def serialConnect(self, port, baud):
      try:
        if self.ser.isOpen():
          return
                
        self.ser.port = port
        self.ser.baudrate = baud
        self.ser.open()
      except Exception as e:
        print(e)

    def send_sample(self, name, timestamp, value):
        if not self.connected:
            self.lnpConnect()
            return

        try:
            self.client.send_sample(name, timestamp=timestamp, value=float(value))
        except:
            self.connected = False

    def process(self):
        self.updates += 1

        self.packetizer.update()

        self.msgCount=0
        self.msgBadCount=0
        self.crcErrors=0


reader = DataReader()


def readerThread():
    while True:
        s = time.time_ns()
        reader.process()

        delta = time.time_ns() - s
        sleep = (1.0 / 60.0) * 1e9 - delta

        reader.send_sample(
            "proc.time.delta", timestamp=float(reader.updates), value=float(delta / 1e9)
        )

        if not commands.empty():
            command = commands.get()
            if command == cmd_quit:
                return

        if sleep > 0:
            time.sleep(sleep / 1e9)

def tabulateValue(v):
  print( tabulate(v if isinstance(v, list) else [v], headers=["Id", "Name", "Value", "Samples", "Type", "Mods Allowed", "Description"]))

class CommandPrompt(Cmd):
    prompt = "> "
    intro = "Welcome! Type ? to list commands"

    def do_exit(self, inp):
        print("Bye")
        return True

    def help_exit(self):
        print("exit the application. Shorthand: x q Ctrl-D.")

    def do_list(self, inp):
        l = self.findValue(inp)

        tabulateValue(l)

    def help_list(self):
        print("list or search all samples")

    def findValue(self, search, exact=False):
        l = []
        for (k, v) in sorted(reader.descriptions.items()):
            if search == v.name if exact else search in v.name:
                value = None
                if k in reader.samples:
                    value = reader.samples[k].data

                r = [k, v.name, value, reader.sampleCounts[k], v.type, v.modsAllowed, v.desc]
                l.append(r)

        return l
      
    def do_status(self, inp):
      print(f"lognplot connected: {reader.connected}")
      print(f"serial connected: {reader.ser.is_open if reader.ser else False}")
      print(f"samples: {len(reader.samples)}")

    def do_serial(self, inp):
      cmds = inp.split()
      
      # serial open /dev/ttyACM0 115200
      if cmds[0] == 'open':
        if len(cmds) > 2:
          reader.serialConnect(cmds[1], cmds[2])
        elif len(cmds) > 1:
          reader.serialConnect(cmds[1], 115200)
        else:
          reader.serialConnect('/dev/ttyACM0', 115200)
          
      elif cmds[0] == 'close':
        reader.ser.close()

    def default(self, inp):
        if inp == "x" or inp == "q":
            return self.do_exit(inp)

        command = inp
        commands.put(command)

        if command == cmd_quit:
            return

        if command in reader.sampleNames:
            l = self.findValue(command, exact=True)
            tabulateValue(l)
            return

        #  check for set command
        match = setre.match(command)
        if match:
            groups = match.groups()
            if len(groups) == 2:
                name = groups[0]
                value = groups[1]
                
                self.setDataValue(name, value)
            return

        self.do_list(command)
        # all failed so search current sample desc for names contain command
        
    def setDataValue(self, name, value):
      if not (name in reader.sampleNames):
        return
      
      id = reader.sampleNames[name]
      
      meta = reader.descriptions[id]
      
      val = float(value) if meta.type == 8 else int(value)
      print('val', val, 't:', meta.type)
      frame = {
        'id': 0,
        'mod': {
          'type': 2,
          'time': 0,
          'value':{
            'id': id,
            'type': meta.type,
            'data':val
          }
        }
      }
      
      reader.packetizer.sender.send(telem.MsgTypes.MOD, frame)
      
      
    def emptyline(self):
      self.do_list("")
  
    do_EOF = do_exit
    help_EOF = help_exit


def consoleInputThread():
    CommandPrompt().cmdloop()
    commands.put(cmd_quit)


if __name__ == '__main__':
    rt = threading.Thread(target=readerThread)
    ct = threading.Thread(target=consoleInputThread)

    rt.start()
    ct.start()
