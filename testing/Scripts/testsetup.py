import socket
import sys

# Keep track of the test case number so that any error message is more useful.
testnum = 0


def testcase(success, errmsg):
    global testnum
    testnum += 1

    if not success:
        sys.stderr.write(f"Test #{testnum}: {errmsg}\n")
        sys.exit(1)


def ipv4binary(ipstr):
    return socket.inet_pton(socket.AF_INET, ipstr)


def ipv6binary(ipstr):
    return socket.inet_pton(socket.AF_INET6, ipstr)
