#!/usr/bin/python
#
# this will generate a random frsky compatible
# hop table and a random txid
#
import random
import textwrap

random.seed()

#get a random number for the txid
txid = random.randint(513, 65000)

hoptable_ok = 0

#generate hoptable
hoptable_ok = 0
while (hoptable_ok == 0):
    #get random numbers for the hoptable calculation
    channel_start   = random.randint(0, 7) 
    channel_spacing = random.randint(64, 255-64)

    hoptable = []
    hop = channel_start
    for i in range(47):
        hoptable.append(hop)
        hop = (hop + channel_spacing) % 235
        if (hop == 0) or (hop == 0x5A) or (hop == 0xDC):
            hop = hop + 1

    #verify that there is no hop more than one time
    hoptable_ok = 1
    for x in range(235):
        count = hoptable.count(x)
        if (count > 1):
            #invalid hoptable, try again
            #print hoptable.count(x)
            hoptable_ok = 0

#great, valid hoptable found
hoptable_s = (", ".join(hex(x) for x in hoptable))

print("/*");
print("    Copyright 2016 fishpepper <AT> gmail.com");
print("");
print("    This program is free software: you can redistribute it and/or modify");
print("    it under the terms of the GNU General Public License as published by");
print("    the Free Software Foundation, either version 3 of the License, or");
print("    (at your option) any later version.");
print("");
print("    This program is distributed in the hope that it will be useful,");
print("    but WITHOUT ANY WARRANTY; without even the implied warranty of");
print("    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
print("    GNU General Public License for more details.");
print("");
print("    You should have received a copy of the GNU General Public License");
print("    along with this program.  If not, see <http://www.gnu.org/licenses/>.");
print("");
print("    author: fishpepper <AT> gmail.com");
print("*/");
print("#ifndef HOPTABLE_H_");
print("#define HOPTABLE_H_");
print("");
print("#define FRSKY_DEFAULT_FSCAL_VALUE 0x00");
print("");
print("#define FRSYK_TXID (0x%04X)" % (txid))
print("")
print("// hoptable was generated with start=%d, spacing=%d" % (channel_start, channel_spacing))
print("#define FRSKY_HOPTABLE { \\")
for x in textwrap.wrap(hoptable_s):
   print("%s \\" % (x))
print("};");       
print("")
print("#endif  // HOPTABLE_H_")
