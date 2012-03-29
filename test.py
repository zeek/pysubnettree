#! /usr/bin/env python

import socket
import struct
import SubnetTree

def unpack(x):
    return struct.unpack("I", x)[0]

t = SubnetTree.SubnetTree()

t.insert("1.2.3.4/16")

if "1.2.3.255" in t:
    print "yes [correct]"
else:
    print "no [incorrect]"

if unpack(socket.inet_aton("1.2.3.255")) in t:
    print "yes [correct]"
else:
    print "no [incorrect]"
    
if "1.3.3.255" in t:
    print "yes [incorrect]"
else:
    print "no [correct]"

if unpack(socket.inet_aton("1.3.3.255")) in t:
    print "yes [incorrect]"
else:
    print "no [correct]"
    
if unpack(socket.inet_aton("0.1.2.3")) in t:
    print "yes [incorrect]"
else:
    print "no [correct]"

t["4.3.2.1/8"] = "indexing works [correct]"
print t["4.3.255.255"]

try:
    print t["42.42.42.42"],
    print "[incorect]"
except KeyError:
    print "catching lookup failure [correct]"

del t["4.3.2.1/8"]

try:
    print t["4.3.2.1/8"],
    print "[incorect]"
except KeyError:
    print "del works [correct]"

try:
    t["a.b.c.d/error"] = 5
except ValueError:
    print "catching parsing errors [correct]"

try:
    print None in t,
    print "should not be printed [incorrect]"
except TypeError:
    print "catching None lookup [correct]"
    
try:    
    t[None] = "should fail"   
    print "should not be printed [incorrect]"
except TypeError:
    print "catching None assignment [correct]"
    
try: 
    print t[None], 
    print "should not be printed [incorrect]"
except TypeError:
    print "catching None access [correct]"

try: 
    del t[None],
    print "should not be printed [incorrect]"
except TypeError:
    print "catching None delete [correct]"

try: 
    print t[42],
    print "should not be printed [incorrect]"
except TypeError:
    print "catching integer lookup [correct]"

# ipv6 tests
t.insert("1:2:3:4::/64")

if "1:2:3:4::5" in t:
    print "yes [correct]"
else:
    print "no [incorrect]"

if "1:3:3:4::5" in t:
    print "yes [incorrect]"
else:
    print "no [correct]"

t["2:3::/32"] = "ipv6 indexing works [correct]"
print t["2:3::1"]

print "switching to binary mode"
t.set_binary_lookup_mode()

if socket.inet_aton("1.2.3.255") in t:
    print "yes [correct]"
else:
    print "no [incorrect]"

if socket.inet_aton("1.3.3.255") in t:
    print "yes [incorrect]"
else:
    print "no [correct]"

if socket.inet_pton(socket.AF_INET6, "1:2:3:4::5") in t:
    print "yes [correct]"
else:
    print "no [incorrect]"

if socket.inet_pton(socket.AF_INET6, "1:3:3:4::5") in t:
    print "yes [incorrect]"
else:
    print "no [correct]"

try:
    print "1" in t
    print "should not be printed [incorrect]"
except ValueError:
    print "caught ascii lookup when in binary mode [correct]"
    
