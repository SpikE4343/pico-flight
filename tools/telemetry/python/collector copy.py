from collections import defaultdict
from tabulate import tabulate

import serial
import time

from lognplot.client import LognplotTcpClient

import telemetry as telem

import threading
import queue
import re

from cmd import Cmd

setre = re.compile("(.*?)\s*=\s*(\d+|true|false)")
cmd_quit = "quit"
commands = queue.Queue()
class DataReader:
    def __init__(self):

        self.client = LognplotTcpClient()
        self.connected = False
        self.lnpConnect()

        self.serialConnect(port="/dev/ttyACM0", baud=115200)

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

        telem.data_frame_packet.stream2.readSize = self.inputWaitingData

        self.data_frame_packet = telem.data_frame_packet.compile()

    def lnpConnect(self):
        try:
            self.client.connect()
            self.connected = True
        except:
            pass
          
    def serialConnect(self, port, baud):
      try:
          self.ser = serial.Serial(port=port, baudrate=baud, stopbits=1)
      except Exception as e:
          print(e)
          self.ser = None

    def send_sample(self, name, timestamp, value):
        if not self.connected:
            self.lnpConnect()
            return

        try:
            self.client.send_sample(name, timestamp=timestamp, value=float(value))
        except:
            self.connected = False

    def inputWaitingData(self):
        if self.ser:
            waiting = self.ser.inWaiting()
            # print(waiting)
            return waiting
        return 16

    def process(self):
        self.updates += 1

        waiting = self.inputWaitingData()

        self.send_sample(
            "serial.waiting.in", timestamp=time.time(), value=float(waiting)
        )

        if waiting < 128:
            return

        msgCount = 0
        msgBadCount = 0
        crcErrors = 0
        parse = 10

        while parse > 0:
            try:
                msg = self.data_frame_packet.parse_stream(self.ser)

                if msg.header.type == "MOD":
                    id = msg.data.payload.value.mod.value.id
                    value = msg.data.payload.value.mod.value
                    st = msg.data.payload.value.mod.time
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
                    msgCount += 1
                elif msg.header.type == "DESC":
                    self.descriptions[
                        msg.data.payload.value.id
                    ] = msg.data.payload.value.meta
                    self.sampleNames[
                        msg.data.payload.value.meta.name
                    ] = msg.data.payload.value.id
                else:
                    msgBadCount += 1

            except StreamError as se:
                pass

            except ConstError as ce:
                # print("ConstError:", ce)
                pass
            except ChecksumError as cse:
                print(cse)
                crcErrors += 1
            except Exception as e:
                print("Error:", e)
                parse = 0
            except EOFError:
                print("EOF")
                parse = 0
            finally:
                parse -= 1

        self.send_sample(
            "reader.msg.valid", timestamp=time.time(), value=float(msgCount)
        )

        self.send_sample(
            "reader.msg.invalid", timestamp=time.time(), value=float(msgBadCount)
        )

        self.send_sample(
            "reader.msg.err.crc", timestamp=time.time(), value=float(crcErrors)
        )


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
  print( tabulate(v if isinstance(v, list) else [v], headers=["Id", "Name", "Value", "Type", "Mods Allowed", "Description"]))

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

                r = [k, v.name, value, v.type, v.modsAllowed, v.desc]
                l.append(r)

        return l
      
    def do_status(self, inp):
      print(f"lognplot connected: {reader.connected}")
      print(f"serial connected: {reader.ser.is_open if reader.ser else False}")
      print(f"samples: {len(reader.samples)}")

    def do_serial(self, inp):
      if inp == 'connect':
        reader.ser.

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

            print("matched:", groups)
            print(match)
            return

        self.do_list(command)
        # all failed so search current sample desc for names contain command
        
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
