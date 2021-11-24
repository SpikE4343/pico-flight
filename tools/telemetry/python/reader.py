from collections import defaultdict
import sys
from typing import BinaryIO
from construct import *

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

  def inputWaitingData(self):
    # if self.ser:
      # return self.ser.inWaiting()
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
        msg =   telem.data_frame_packet.parse_stream(self.ser)
        
        # TODO: could go past tell? - generates a StreamError need to roll stream back to tell
        
        if msg.header.type == "MOD":
          id = msg.data.payload.value.mod.value.id
          value = msg.data.payload.value.mod.value
          st = msg.data.payload.value.mod.time
          fst = float(st)
          
          # TODO: check against value table
          if(id < 20):      
            self.samples[id] = value
            self.sampleCounts[id] += 1
            
            if( self.sampleTime[id] >= fst):
              print("Time rolled back:", msg.data.payload.value.id, fst-self.sampleTime[id], fst, self.sampleTime[id], id )
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