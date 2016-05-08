import subprocess
import threading
import sys, os, inspect
import httplib
import urllib2
import datetime
import signal
sys.path.append('/home/dashboard/codebase/link/utils')
sys.path.append('/Users/prettywise/codebase/Codebase/link/utils')
import parse
import link

def main():
  thread, parser = link.run("", sys.argv[1:]) 

  result = parser.ReadToLine("^.*(rest server listening on port:).*$")
  if(result == None):
    raise Exception("failed to obtain rest listen port")

  port_no = parse.FindInteger(result);

  result = parser.ReadToLine("^.*(msg=command command-list registration succeeded).*$")
  if(result == None):
    raise Exception("failed to register command")

  if(port_no == None):
    raise Exception("failed to find rest listen port")

  print("rest liten port: " + str(port_no))

  url = "http://127.0.0.1:" + str(port_no) + "/command-list";
#  try:
  response = urllib2.urlopen(url)
  print(url + " resulted in (" + str(response.getcode()) + "): " + response.read())


  if response.getcode() != 200:
    raise Exception(url + " failed with: " + str(response.getcode()))

#  except:
#    print("error because the command-list was not registered yet. need to fix that.")

  parser.Kill()
  parser.Wait();
  thread.join(10)

if __name__ == '__main__': main()
