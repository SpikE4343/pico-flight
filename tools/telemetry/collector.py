from collections import defaultdict
import sys
from typing import BinaryIO
from construct import *
from construct.core import Float32l, Int32ul, Int32sl, Int8ul, Int8sl, Int16ul, Int16sl
import hashlib

import serial
import io
import time

from lognplot.client import LognplotTcpClient

from dearpygui.core import *
from dearpygui.simple import *
import telemetry as telem

class DataReader:
  def __init__(self):
    self.client = LognplotTcpClient()
    self.client.connect()

    self.baud = 115200
    
    try:
      self.ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, stopbits=1)
    except Exception as e:
      print(e)
      self.ser = None

    self.samples = {}
    self.sampleCounts = defaultdict(int)
    self.sampleTime = defaultdict(float)
    self.descriptions = {}

    self.dumpOnly = False
    # self.dumpFile = open("data-out.bin", "wb")
    # self.file = open("data-in.bin", "rb")

    self.updates = 0
    
    self.readpos = 0
    self.crcErrors = 0
    
    telem.data_frame_packet.stream2.readSize = self.inputWaitingData
    
    
    self.data_frame_packet = telem.data_frame_packet.compile()

  def inputWaitingData(self):
    if self.ser:
      waiting = self.ser.inWaiting()
      # print(waiting)
      return waiting
    return 16

  def process(self):
    self.updates += 1
    
    waiting = 16 #self.inputWaitingData()
    
    self.client.send_sample("serial.waiting.in", 
      timestamp=time.time(), 
      value=float(waiting)
      )
          
    msgCount = 0
    msgBadCount = 0
    crcErrors = 0
    parse = 10
                          
    while parse > 0:
      try:
        msg =   self.data_frame_packet.parse_stream(self.ser)
        
        if msg.header.type == "MOD":
          id = msg.data.payload.value.mod.value.id
          value = msg.data.payload.value.mod.value
          st = msg.data.payload.value.mod.time
          fst = float(st)
          
          self.samples[id] = value
          self.sampleCounts[id] += 1
          
          if( self.sampleTime[id] > fst):
            print("Time rolled back:", id, fst-self.sampleTime[id], fst, self.sampleTime[id], id )
          else:
            meta = self.descriptions.get(value.id)
            if meta is None:
              name = value.id
            else:
              name = meta.name
              
            self.client.send_sample( str(name), timestamp=fst, value=float(value.data))
          
          self.sampleTime[id] = fst
          msgCount += 1
        elif msg.header.type == "DESC":
          self.descriptions[msg.data.payload.value.id] = msg.data.payload.value.meta
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
      
    self.client.send_sample(
      "reader.msg.valid", 
      timestamp=time.time(), 
      value=float(msgCount)
      )
    
    self.client.send_sample(
      "reader.msg.invalid", 
      timestamp=time.time(), 
      value=float(msgBadCount)
      )
    
    self.client.send_sample(
      "reader.msg.err.crc", 
      timestamp=time.time(), 
      value=float(crcErrors)
      )
        

reader = DataReader()

if len(sys.argv) > 1 and sys.argv[1] == "gui":

  with window("Telemetry"):
    with tab_bar("tab##Data"):
      with tab("Device"):
        add_text("Data")
        add_text("reader.updates", default_value=str(0))
        add_label_text("serial.waiting", default_value=str(0))
        add_label_text("stream.read.pos", default_value=str(0))
        add_label_text("stream.read.size", default_value=str(0))
        add_label_text("stream.write.pos", default_value=str(0))
        add_label_text("stream.write.size", default_value=str(0))
      
  def main_callback(sender, data):
    #log_debug(reader)
    reader.process()
    
  set_render_callback(main_callback)    

  show_logger()
  # show_debug()
  # show_documentation()


  enable_docking(dock_space=True)
  start_dearpygui() #primary_window="Telemetry")
else:
  while True:
    s = time.time_ns()
    reader.process()
    
    delta = time.time_ns() - s
    sleep = ((1.0/60.0)*1e9 - delta)
    
    reader.client.send_sample(
      "proc.time.delta", 
      timestamp=float(reader.updates), 
      value=float(delta/1e9)
      )
    
    if(sleep > 0):
      time.sleep(sleep/1e9)
