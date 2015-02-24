#!/usr/bin/python

#
#    Copyright (C) 2014 Tamas Szabo <sza2trash@gmail.com>
#    Copyright (C) 2015 Mikhail Grigorev <sleuthhound@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import serial
import argparse
import time

class nodemcu:
	def __init__(self, device = 0, baud = 9600, tout = 3):
		self.opened = True
		if args.verbose: print 'Connecting...'
		try:
			self.device = serial.Serial(device, baud, timeout = tout)
		except IOError:
			self.opened = False
			return None

		if args.reset: 
			if args.verbose: print 'Reset device...'
			self.device.setRTS(True)		#RTS
			time.sleep(0.25)
			self.device.setRTS(False)
			time.sleep(0.2)

		if args.verbose: print 'Flush device...'
		self.device.flush()
		self.chunk_size = 32 # number of bytes transfered in one file.write() call

	def write(self, data):
		if args.verbose: print 'Sending request...'
		self.device.write(data + "\r") # NodeMCU uses <CR> as line end
		if args.verbose: print 'Reading answer...'
		self.device.read(len(data) + 1) # read back the sent characters (echoed)
		self.device.read() # read and drop leading linefeed

	def read(self):
		if args.verbose: print 'Reading data...'
		found_eod = False
		data = ""
		eod1 = "> " # prompt can be "> "
		eod2 = ">> " # or ">> "
		while(not found_eod):
			data += self.device.read()
			if eod1 in data or eod2 in data:
				found_eod = True
		data = data.split("\r\n")
		del data[-1] # remove return prompt from the list
		return data

	def execute(self, command):
		self.write(command)
		return self.read()

	def file_list(self):
		self.execute("filelist = file.list()")
		self.execute("for filename, filesize in pairs(filelist) do")
		self.execute("print(filesize .. \" \" .. filename)")
		files = self.execute("end")
		return files # list contains the size and name of the files

	def file_exists(self, filename):
		self.execute("f = file.open(\"" + filename + "\", \"r\")")
		result = self.execute("print(f)")[0] # if file.open does not return 'nil' then the file exists

		if result == "true":
			self.execute("file.close()")
			return True
		return False

	def file_remove(self, filename):
		if self.file_exists(filename):
			self.execute("file.remove(\"" + filename + "\")")
			return True
		else:
			return False

	def file_read_from_node(self, filename):
		if self.file_exists(filename):
			self.execute("file.open(\"" + filename + "\")")
			fsize = self.execute("print(file.seek(\"end\", 0))")[0]
			self.execute("file.seek(\"set\", 0)")
			self.execute("for cnt = 1, " + fsize + " do")
			self.execute("d = file.read(1)")
			self.execute("uart.write(0, d)")
			self.write("end")
			file_content = ""
			for cnt in range(0, int(fsize)):
				file_content +=self.device.read(1)
			self.read()
			self.execute("file.close()")
			return file_content
		return False

	def file_write_to_node(self, filename, data):
		if self.file_exists(filename):
			self.file_remove(filename) # remove the file is already exists
		self.execute("file.open(\"" + filename + "\", \"w\")")
		for chunk in range(len(data) / self.chunk_size + 1):
			file_content = ""
			for d in data[chunk * self.chunk_size:(chunk + 1) * self.chunk_size]:
				file_content += "\\" + str(ord(d)) # convert the binary value to escape format ("\nnn")
			self.execute("file.write(\"" + file_content + "\")")
		self.execute("file.close()")
		return True

	def delete_file(self, filename):
		print 'Delete file from flash...'
		if self.file_remove(filename):
			print "%s deleted" % filename
			return True
		else:
			print "%s not found" % filename
			return False

	def write_to_node(self, filename, node_filename):
		print 'Transfer file from host to flash...'
		try:
			f = open(filename, "rb")
		except IOError as e:
			print "%s does not exist" % filename
			return False
		data = f.read()
		f.close()
		self.file_write_to_node(node_filename, data)
		return True

	def read_from_node(self, filename, node_filename):
		print 'Transfer file from flash to host...'
		if(not self.file_exists(node_filename)):
			print "%s does not exist on node" % node_filename
			return False
		try:
			f = open(filename, "wb")
		except IOError as e:
			print "cannot write %s" % filename
			return False
		data = self.file_read_from_node(node_filename)
		f.write(data)
		if data == False:
			print "cannot read content from node"
			return False
		f.close()
		return True

	def print_file_list(self):
		print 'List flash filesystem files...'
		f = self.file_list()
		print "%d file(s) found:" % len(f)
		for fil in f:
			fil = fil.split(" ")
			print "%5d %s" % (int(fil[0]), fil[1])

	def execute_file(self, filename):
		print 'Execute the specified file...'
		self.write("dofile(\"" + filename + "\")")

	def restart(self):
		print 'Execute command node.restart()...'
		self.write("node.restart()")

if __name__ == '__main__':
	# parse arguments or use defaults
	parser = argparse.ArgumentParser(description='NodeMCU file transfer utility')
	parser.add_argument('-p', '--port',    default='/dev/ttyUSB0', help='Device name, default /dev/ttyUSB0')
	parser.add_argument('-b', '--baud',    default=9600,           help='Baudrate, default 9600')
	parser.add_argument('-v', '--verbose', action='store_true',    help="Show progress messages.")
	parser.add_argument('-rs', '--reset', action='store_true',    help="Send signal RTS to reset device.")
	group = parser.add_mutually_exclusive_group()
	group.add_argument('-s', '--send',    help='Transfer file from host to flash')
	group.add_argument('-r', '--receive', help='Transfer file from flash to host')
	group.add_argument('-l', '--list',    action="store_true",     help='List flash filesystem files')
	group.add_argument('-d', '--delete',  help='Delete file from flash')
	group.add_argument('-e', '--execute', help='Execute the specified file')
	group.add_argument('-rt', '--restart', action="store_true", help='Execute node.restart() command')
	args = parser.parse_args()

	node = nodemcu(args.port, args.baud)

	if not node.opened:
		print "%s: unable to open port" % args.port
		exit(1)

	if args.list:
		node.print_file_list()

	if args.delete:
		node.delete_file(args.delete)

	if args.receive:
		node.read_from_node(args.receive, args.receive)

	if args.send:
		node.write_to_node(args.send, args.send)

	if args.execute:
		node.execute_file(args.execute)

	if args.restart:
		node.restart()
