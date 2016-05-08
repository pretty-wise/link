import re
import datetime
import sys
import os
import time
import signal

def FindInteger(string):
  for i, s in enumerate(string.split()):
    if(s.isdigit() and i != 0): # 0 index is time.
      return s
  return None

class OutputProcessor:

  def __init__(self, process):
    self.process = process
    self.read_buffer = []
    # wait for the process to start.

  def ReadToLine(self, regex):
    while self.process.poll() is None:
      # check in what we already read.
      for line in self.read_buffer:
#        sys.stdout.write(line)
        if(re.match(regex, line) != None ):
          return line
      time.sleep(0.01)
    return None

  def ReadAll(self, prefix):
    while True:
      self.process.poll()
      line = self.process.stdout.readline()
      if line != '':
        #the real code does filtering here
        self.read_buffer.append(line)
        print prefix + " " + line.rstrip()
      else:
        break
      time.sleep(0.01)

  def Kill(self):
    if(self.process.returncode == None):
      os.kill(self.process.pid, signal.SIGINT)
    print "\n"

  def Wait(self):
    self.process.wait()
    print("return code: " + str(self.process.returncode))

    if(self.process.returncode != 0):
      raise Exception("process returned: " + str(self.process.returncode))
    
    if(self.process.returncode != 0):
      raise Exception("endpoint B returned: " + str(self.process.returncode))

  def FindTcpPort(self, plugin_name):
    result = self.ReadToLine("^.*"+plugin_name+"\(.*\)( listening on port).*$")
    if(result == None):
      raise Exception("failed to obtain listen port")
    port = FindInteger(result)
    return port

  def FindRestPort(self):
    result = self.ReadToLine("^.*(rest server listening on port:).*$")
    if(result == None):
      raise Exception("failed to obtain rest listen port")
    port = FindInteger(result)
    return port
