import threading
import os
import sys
import re
import parse
import subprocess

def thread_main(name, parser):
  print "thread_main start: " + name
  parser.ReadAll(name)
  print "thread_main end: " + name

def run(name, argv):
  process = subprocess.Popen(argv, cwd=os.getcwd(), stdout=subprocess.PIPE)
  parser = parse.OutputProcessor(process)
  thread = threading.Thread(target=thread_main, args=(name, parser))
  thread.start()
  parser.ReadToLine("^.*(starting link server).*$")
  return thread, parser
