#! /usr/bin/python
# -*- coding: utf-8 -*-

# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA


# Creates a bin of the specified directory, named dirname.bin
# Each filename cannot exceed 256 characters
# Each file cannot exceed 65536 bytes
# Following formats are further processed removing unquoted controls characters (spaces, CR, LF, etc..):
#   tpl, html, htm, xml, css
#
# At the top of the bin a directory of the existing files is inserted. It has following structure:
#   uint32_t: MAGIC = 0x93841A03
#   file entries:
#     uint8_t:  filename length including terminating zero (0 = End of Files)
#     uint8_t:  mime type length including terminating zero
#     uint16_t: file content length
#     x-bytes:  filename data + zero
#     x-bytes:  mime type data + zero
#     x-bytes:  raw file data
# All values are little-endian ("<" in the struct.pack calls)
#
# To optimize html, css, js this script can use "slimmer". Just install it with:
#   easy_install slimmer


import sys
import os
import mimetypes
import struct
import glob
from pkgutil import iter_modules


def module_exists(module_name):
    return module_name in [tuple_[1] for tuple_ in iter_modules()]

	
do_slimmer = module_exists("slimmer")
if do_slimmer:
	import slimmer


if len(sys.argv) != 4:
	print "usage:"
	print "  binarydir.py dirpath outfilename maxsize"
	exit()

dirpath = sys.argv[1]
files = glob.glob(os.path.join(dirpath, "*.*"))
#print files
dirname = os.path.basename(dirpath)
outfilename = sys.argv[2]

with open(outfilename, "wb") as fw:
	# magic
	fw.write(struct.pack("<I", 0x93841A03))
	# loop among files
	for filepath in files:
		filename = os.path.basename(filepath)
		mimetype = mimetypes.guess_type(filepath, strict = False)[0].encode('ascii','ignore')
		fileext = os.path.splitext(filename)[1].lower()		
		if not mimetype:
			# try to handle additional types unknown to mimetypes.guess_type()			
			if fileext == ".tpl":
				mimetype = "text/html"
			else:
				mimetype = "application/octet-stream"
				
		# get raw file data
		with open(filepath, "rb") as fr:
			filedata = fr.read()
		
		oldfilesize = len(filedata)
		
		# can I remove CR, LF, Tabs?
		if do_slimmer:			
			if fileext in [".tpl", ".html", ".htm"]:
				filedata = slimmer.html_slimmer(filedata)
			elif fileext in [".css"]:
				filedata = slimmer.css_slimmer(filedata)
			elif fileext in [".js"]:
				filedata = slimmer.js_slimmer(filedata)			

		print "Adding {} mimetype = ({}) size = {}  reduced size = {}".format(filename, mimetype, oldfilesize, len(filedata))
				
		# filename length, mime tpye length, file content length
		fw.write(struct.pack("<BBH", len(filename) + 1, len(mimetype) + 1, len(filedata)))
				
		# filename data
		fw.write(struct.pack(str(len(filename)) + "sB", filename, 0x00))
		
		# mime type data
		fw.write(struct.pack(str(len(mimetype)) + "sB", mimetype, 0x00))
		
		# file data
		fw.write(filedata)
		
	# filename length = 0 -> end of files
	fw.write(struct.pack("B", 0))
	
outsize = os.path.getsize(outfilename)
maxsize = int(sys.argv[3])
print "out = {} bytes   max = {} bytes".format(outsize, maxsize)
print ""
if outsize > maxsize:
	print "Error! exceeded max file size."
	sys.exit(1)


		
		
	
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
