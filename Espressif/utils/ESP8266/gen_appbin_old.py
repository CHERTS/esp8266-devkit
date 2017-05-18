#!/usr/bin/python
#
# File	: gen_appbin.py
# This file is part of Espressif's generate bin script.
# 
# ESPRESSIF MIT License
# 
# Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
# 
# Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
# it is free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the Software is furnished
# to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
# 

import string
import sys
import os
import re

if len(sys.argv) != 3:
    print 'Usage: gen_appbin.py eagle.app.out version'
    sys.exit(0)

elf_file = sys.argv[1]
ver = sys.argv[2]
#print elf_file

cmd = 'C:\\Espressif\\xtensa-lx106-elf\\bin\\xtensa-lx106-elf-nm.exe -g ' + elf_file + ' > eagle.app.sym'
#print cmd
os.system(cmd)

fp = file('./eagle.app.sym')
if fp is None:
    print "open sym file error\n"
    exit

lines = fp.readlines()

fp.close()

entry_addr = None
p = re.compile('(\w*)(\sT\s)(call_user_start)$')
for line in lines:
    m = p.search(line)
    if m != None:
        entry_addr = m.group(1)
        #entry_addr = int(entry_addr, 16)
        print entry_addr

if entry_addr is None:
    print 'no entry point!!'
    exit

data_start_addr = '0'
p = re.compile('(\w*)(\sA\s)(_data_start)$')
for line in lines:
    m = p.search(line)
    if m != None:
        data_start_addr = m.group(1)
        print data_start_addr

rodata_start_addr = '0'
p = re.compile('(\w*)(\sA\s)(_rodata_start)$')
for line in lines:
    m = p.search(line)
    if m != None:
        rodata_start_addr = m.group(1)
        print rodata_start_addr

cmd = 'C:\\Espressif\\utils\genflashbin%s eagle.app.%s.text.bin '%(ver, ver)+entry_addr+' eagle.app.%s.data.bin '%(ver)+ data_start_addr+' eagle.app.%s.rodata.bin '%(ver)+rodata_start_addr

print cmd
os.system(cmd)

cmd = 'ren eagle.app.flash.bin eagle.app.%s.flash.bin'%(ver)

print cmd
os.system(cmd)
