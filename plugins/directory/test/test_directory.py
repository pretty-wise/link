import subprocess
import threading
import sys, os, inspect
import httplib
import urllib2
import datetime
import signal
import time
sys.path.append('/home/dashboard/codebase/link/utils')
sys.path.append('/Users/prettywise/codebase/Codebase/link/utils')
import parse
import link
import json

def find_tcp_port(processor):
  result = processor.ReadToLine("^.*(listening on port).*$")
  if(result == None):
    raise Exception("failed to obtain listen port")
  port = parse.FindInteger(result)
  return port

def find_plugin_info(processor):
  result = processor.ReadToLine("^.*(plugin registration succeeded. name: directory).*$")
  if(result == None):
    raise Exception("failed to obtain plugin info")
  str_list = result.split()
  name = str_list[7].rstrip(',')
  version = str_list[9].rstrip(',')
  hostname = str_list[11].rpartition(':')[0]
  port = str_list[11].rpartition(':')[2].rstrip(',')
  pid = str_list[13].rstrip(',')
  return name, version, hostname, port, pid

def http_request(hostname, port, command, data):
  url = "http://" + hostname + ":" + str(port) + command
  content = None
  print("local request: " + url)
  try:
    response = urllib2.urlopen(url, data)
    content = response.read()
    print(url + " resulted in (" + str(response.getcode()) + "): " + content)
    if response.getcode() != 200:
      raise Exception(url + " failed with: " + str(response.getcode()))
  except:
    print("404 probably")
  return content

def get_command(hostname, port, command):
  response = http_request(hostname, port, "/command-list", None)
  if response == None:
    return None
  commands_json = json.loads(response)
  for cmd in commands_json['commands']:
    if command in cmd:
      return cmd
  return None

def main():
  #hostname = "127.0.0.1";
  #hostname = "178.62.205.199"
  #hostname = "code.krzysiekstasik.com"
  hostname = "localhost"
  #hostname = "192.168.1.50"

  link_path = sys.argv[1]
  plugin_path = sys.argv[2]
  rest_plugin = sys.argv[3]
  master_config = "--config=<link hostname=\"" + hostname + "\"><plugin path=\"" + plugin_path + "\"/><plugin path=\"" + rest_plugin + "\"/></link>"

  print plugin_path
  print master_config
  master_thread, master_parser = link.run("master", [link_path, master_config]) 
  
  master_port = find_tcp_port(master_parser)
  print "master on port " + str(master_port)

  master_name, master_version, master_hostname, master_port, master_pid = find_plugin_info(master_parser)
  print("plugin info: " + master_name + "(" + master_version + ") " + master_hostname + ":" + str(master_port) + " pid: " + str(master_pid))

  slave_config = "--config=<link hostname=\"" + hostname + "\"><plugin path=\"" + plugin_path + "\"><config><master name=\"" + master_name + "\" version=\"" + master_version + "\" hostname=\"" + hostname + "\" port=\"" + str(master_port) + "\" pid=\"" + str(master_pid) + "\"/></config></plugin></link>"
  slave_thread, slave_parser = link.run("slave ", [link_path, slave_config])

  time.sleep(1)

  master_rest_port = master_parser.FindRestPort()
  print("master rest port: " + str(master_rest_port))

#  slave_rest_port = slave_parser.FindRestPort()
#  print("slave rest port: " + str(slave_rest_port))

#  response = http_request(hostname, master_rest_port, "/command-list", None)
  plugin_list_cmd = get_command(hostname, master_rest_port, "plugin-list")
  print("listing command: " + plugin_list_cmd)

  plugin_list = http_request(hostname, master_rest_port, plugin_list_cmd, None)
  print("result: " + plugin_list)


  time.sleep(1)

  slave_parser.Kill()
  slave_parser.Wait()

  time.sleep(1)

  master_parser.Kill()
  master_parser.Wait()
  master_thread.join(10)
  slave_thread.join(10)

if __name__ == '__main__': main()
