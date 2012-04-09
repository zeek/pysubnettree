#! /usr/bin/env python

import socket
import struct
import SubnetTree

numerr = 0

def testcase(passfail, str):
    global numerr

    if passfail:
        result = "ok"
    else:
        result = "FAILED"
        numerr += 1
    print "%s ... %s" % (str, result)


def unpack(x):
    return struct.unpack("I", x)[0]

###############################################
# Tests when binary mode is not set initially
t1 = SubnetTree.SubnetTree()

t1.insert("1.2.3.4/16", "Network 1.2")
t1["4.3.2.1/8"] = "Network 4"
t1.insert("1:2:3:4::/64")
t1.insert("1:2:3:7::/64", "Network 1:2, subnet 1:2:3:7")
t1.insert("1:2::/32", "Network 1:2")
t1["2:3::/32"] = "Network 2:3"
t1["::ffff:0:0/96"] = "IPv4-mapped addrs"

testcase(not t1.get_binary_lookup_mode(), "Verify that binary lookup mode is not set")

# verify that lookups of matching entries are successful
testcase(t1["4.1.255.255"] == "Network 4", "IPv4 in subnet returns correct value")
testcase("1.2.3.255" in t1, "IPv4 in subnet")
testcase(t1["1.2.3.255"] == "Network 1.2", "IPv4 in subnet returns correct value")
testcase(unpack(socket.inet_aton("1.2.3.255")) in t1, "IPv4 (integer) in subnet")
testcase(t1["2:3::1"] == "Network 2:3", "IPv6 in subnet returns correct value")
testcase(t1["1:2:99::"] == "Network 1:2", "IPv6 in subnet returns correct value")
testcase(t1["1:2:3:7:10::"] == "Network 1:2, subnet 1:2:3:7", "IPv6 in subnet returns correct value")
testcase("1:2:3:4::5" in t1, "IPv6 in subnet")
testcase("::ffff:1.2.3.4" in t1, "IPv4-mapped IPv6 address")

# verify that lookup fails correctly when no match found
testcase("1:3:3:4::5" not in t1, "IPv6 not in subnet")
testcase("1.3.3.255" not in t1, "IPv4 not in subnet")    
testcase(unpack(socket.inet_aton("1.3.3.255")) not in t1, "IPv4 (integer) not in subnet")
try:
    if t1["1.3.3.255"]:  pass
    testcase(False, "Catch IPv4 not in subnet lookup failure")
except KeyError:
    testcase(True, "Catch IPv4 not in subnet lookup failure")

# lookup IP addr (binary) that does not match any entries
testcase(socket.inet_aton("2.1.2.3") not in t1, "IPv4 (binary) not in subnet")

# verify that entries can be deleted
del t1["4.0.0.0/8"]
t1.remove("1:2::/32")

testcase("4.3.2.1" not in t1, "IPv4 in deleted subnet")
testcase("1:2:99::" not in t1, "IPv6 in deleted subnet")
try:
    if t1["4.3.2.1"]:  pass
    testcase(False, "Catch deleted element lookup failure")
except KeyError:
    testcase(True, "Catch deleted element lookup failure")

# verify that adding invalid CIDR subnets fails
try:
    t1["a.b.c.d/24"] = "invalid"
    testcase(False, "Catch IPv4 parsing error")
except ValueError:
    testcase(True, "Catch IPv4 parsing error")
try:
    t1["1.2.3.0/x"] = "invalid"
    testcase(False, "Catch IPv4 parsing error")
except ValueError:
    testcase(True, "Catch IPv4 parsing error")

# verify that attempted use of non-string Python data types are caught
try:
    if None in t1:  pass
    testcase(False, "Catch existence test of None")
except TypeError:
    testcase(True, "Catch existence test of None")
    
try:    
    t1[None] = "should fail"   
    testcase(False, "Catch assignment of None")
except TypeError:
    testcase(True, "Catch assignment of None")
    
try: 
    if t1[None]:  pass
    testcase(False, "Catch lookup of None")
except TypeError:
    testcase(True, "Catch lookup of None")

try: 
    del t1[None]
    testcase(False, "Catch delete of None")
except TypeError:
    testcase(True, "Catch delete of None")

try: 
    if t1[42]:  pass
    testcase(False, "Catch lookup of integer")
except TypeError:
    testcase(True, "Catch lookup of integer")

# switch to binary lookup mode
t1.set_binary_lookup_mode()
testcase(t1.get_binary_lookup_mode(), "Switch to binary lookup mode")

# verify that we can still access previously-added entries
testcase(socket.inet_aton("1.2.3.255") in t1, "IPv4 in subnet (binary mode)")
testcase(socket.inet_pton(socket.AF_INET6, "1:2:3:4::5") in t1, "IPv6 in subnet (binary mode)")

# verify that IP addrs (binary) not in subnet are not found
testcase(socket.inet_aton("1.3.3.255") not in t1, "IPv4 not in subnet (binary mode)")
testcase(socket.inet_pton(socket.AF_INET6, "1:3:3:4::5") not in t1, "IPv6 not in subnet (binary mode)")

# verify that we can add new entries in binary lookup mode
t1["11.1.2.0/24"] = "Network 11.1.2"
testcase(t1[socket.inet_aton("11.1.2.3")] == "Network 11.1.2", "IPv4 in subnet returns correct value (binary mode)")

# verify that IP text string lookups of existing entries fails in binary mode
try:
    if "11.1.2.3" in t1:  pass
    testcase(False, "Catch IPv4 string in subnet lookup failure (binary mode)")
except ValueError:
    testcase(True, "Catch IPv4 string in subnet lookup failure (binary mode)")

# turn off binary lookup mode
t1.set_binary_lookup_mode(False)
testcase(not t1.get_binary_lookup_mode(), "Turn off binary lookup mode")

# verify that we can do string lookups of entries added in binary lookup mode
testcase("11.1.2.3" in t1, "IPv4 string in subnet now works")
testcase(t1["11.1.2.4"] == "Network 11.1.2", "IPv4 in subnet returns correct value")

###############################################
# Tests of setting binary lookup mode initially
t2 = SubnetTree.SubnetTree(True)

t2.insert("10.0.0.0/16")
t2["12.1.2.0/24"] = "Network 12.1.2"
t2["2000:1234::/64"] = "Network 2000:1234"

testcase(t2.get_binary_lookup_mode(), "Verify that binary lookup mode is set")

# verify that binary format lookups in binary mode work correctly
testcase(socket.inet_aton("10.0.1.255") in t2, "IPv4 in subnet (binary mode)")
testcase(t2[socket.inet_aton("12.1.2.5")] == "Network 12.1.2", "IPv4 in subnet lookup returns correct value (binary mode)")
testcase(t2[socket.inet_pton(socket.AF_INET6, "2000:1234::1")] == "Network 2000:1234", "IPv6 in subnet (binary mode)")

# verify that text string lookups in binary mode fail
try:
    if "10.0.1.255" in t2:  pass
    testcase(False, "Catch string IPv4 in subnet lookup failure (binary mode)")
except ValueError:
    testcase(True, "Catch string IPv4 in subnet lookup failure (binary mode)")
try:
    if "2000:1234::1" in t2:  pass
    testcase(False, "Catch string IPv6 in subnet lookup failure (binary mode)")
except ValueError:
    testcase(True, "Catch string IPv6 in subnet lookup failure (binary mode)")

# turn off binary lookup mode
t2.set_binary_lookup_mode(False)
testcase(not t2.get_binary_lookup_mode(), "Turn off binary lookup mode")

# verify that text string lookups of entries added in binary lookup mode work
testcase("10.0.1.255" in t2, "IPv4 in subnet")
testcase(t2["2000:1234::1"] == "Network 2000:1234", "IPv6 in subnet")

###############################################
# Tests of text string lookups in binary lookup mode
t3 = SubnetTree.SubnetTree(True)

t3.insert("2000::/16")
t3.insert("49.58.0.0/16", "Network 49.58")

# verify that passing a 16-character string (a valid IPv6 addr) is interpreted
# as binary format due to being in binary lookup mode
testcase("2000::12:34:1234" not in t3, "16-character IPv6 text string (binary mode)")

# verify that 4-byte strings are interpreted as IPv4 binary format
testcase(t3["1::5"] == "Network 49.58", "4-character IPv6 text string (binary mode)")

# turn off binary lookup mode
t3.set_binary_lookup_mode(False)

# now the lookup succeeds
testcase("2000::12:34:1234" in t3, "16-character IPv6 text string")

# the 4-byte text string is not found (as expected)
testcase("1::5" not in t3, "4-character IPv6 text string")


# Output summary of results
if numerr:
    print "There were %d failures." % numerr
else:
    print "All test cases passed."

