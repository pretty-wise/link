import sys
import socket
import struct
import time

sys.path.append('~/codebase/link/utils')
sys.path.append('/Users/prettywise/codebase/Codebase/link/utils')

import parse
import link

def write_message(socket, message):
	# ! indicates network endianness (big endian)
	packed = struct.pack('!i%ds' % len(message), len(message), message)
	socket.send(packed)
	
def main():
	thread, parser = link.run("", sys.argv[1:]) 

	port_line = parser.ReadToLine("^.*(gate listening on port:).*$")
	if(port_line == None):
		raise Exception("failed to read port line")

	port_no = parse.FindInteger(port_line)
	if(port_no == None):
		raise Exception("failed to read port number")

	print 'gate port is: ' + str(port_no)

	ip = '127.0.0.1'
	message = "test_message"

	sockets = {}
	num_sockets = 4

	for i in range(0,num_sockets):
		sockets[i] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sockets[i].connect((ip, int(port_no)))
		write_message(sockets[i], message + " " + str(i))
		sockets[i].close()
		time.sleep(0.1)

	for i in range(0, num_sockets):
		sockets[i].close()

	time.sleep(0.2)

	parser.Kill()
	parser.Wait();

	thread.join(10)

if __name__ == '__main__': main()
