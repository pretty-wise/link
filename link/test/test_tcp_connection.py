import sys
import os
import subprocess
import threading
import signal
import urllib2
import json
import time
sys.path.append('/home/dashboard/codebase/link/utils')
sys.path.append('/Users/prettywise/Codebase/codebase/link/utils')
import parse
import link

def http_local_request(port, command, data):
  url = "http://127.0.0.1:" + str(port) + command
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

def get_command(port, command):
  response = http_local_request(port, "/command-list", None)
  if response == None:
    return None
  commands_json = json.loads(response)
  for cmd in commands_json['commands']:
    if command in cmd:
      return cmd
  return None

if __name__ == "__main__":
  print("pre main")
  print(sys.argv)
#  main()
  link_A, parser_A = link.run("A", sys.argv[1:])
  link_B, parser_B = link.run("B", sys.argv[1:])

#  endpoint_A_port = parser_A.FindTcpPort("tcp_connect")
#  print("port for endpoint A: " + str(endpoint_A_port))

  endpoint_A_rest_port = parser_A.FindRestPort()
  print("rest port for endpoint A: " + str(endpoint_A_rest_port))
  
  tcp_connect_B_port = parser_B.FindTcpPort("tcp_connect")
  print("tcp_connect port on endpoint B: " + str(tcp_connect_B_port))

  endpoint_B_rest_port = parser_B.FindRestPort()
  print("rest port for endpoint B: " + str(endpoint_B_rest_port))

  parser_A.ReadToLine("^.*(command register registration succeeded).*$")
  parser_A.ReadToLine("^.*(command connect registration succeeded).*$")
  parser_A.ReadToLine("^.*(command command-list registration succeeded).*$")
  parser_B.ReadToLine("^.*(command register registration succeeded).*$")
  parser_B.ReadToLine("^.*(command connect registration succeeded).*$")
  parser_B.ReadToLine("^.*(command command-list registration succeeded).*$")

  print("go!")

  register_cmd_A = get_command(endpoint_A_rest_port, "/register")

  if register_cmd_A:
    print("A's register command: " + register_cmd_A)

    connect_cmd_A = get_command(endpoint_A_rest_port, "/connect")
    print("A's connect command: " + connect_cmd_A)

    # register B's tcp connect in A.
    name = "tcp_connect"
    version = "1.0"
    pid = parser_B.process.pid
    json_request = "{ \"hostname\": \"127.0.0.1\", \"port\": " + str(tcp_connect_B_port) + ", \"name\": \"" + name + "\", \"version\": \"" + version + "\", \"pid\": " + str(pid) + " }"
    ret = http_local_request(endpoint_A_rest_port, register_cmd_A, json_request)
    print("B's tcp_connect registered in A: " + ret)

    json_content = json.loads(ret)
    tcp_connect_B_handle_in_A = json_content['handle']

    # run connection A -> B
    json_request_2 = "{\"handle\": " + str(tcp_connect_B_handle_in_A) + "}"
    ret = http_local_request(endpoint_A_rest_port, connect_cmd_A, json_request_2)
    print("A's tcp_connect connects with B's tcp_connect: " + ret)

    parser_A.ReadToLine("^.*(sent all data, closing connection).*$")
    print("all data sent")
    parser_B.ReadToLine("^.*(all data received, closing connection).*$")
    print("all data received")

#  time.sleep(2)

  parser_A.Kill()
  parser_B.Kill()
 
  parser_A.Wait();
  parser_B.Wait();
 
  link_A.join(10)
  link_B.join(10)

  print("post main")
