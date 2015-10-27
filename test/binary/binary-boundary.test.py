import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedBinaryConnection
from internal.memcached_connection import STATUS, COMMANDS

mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)

def iequal(left, right, level = 1):
    if (left != right):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: %s not equal %s" % (tb[0], tb[1],
                repr(left), repr(right))
        if (isinstance(left, basestring)):
            if (len(left) != len(right)):
                print("length is different")
        return False
    return True

def __check(res, flags, val, level = 0):
    return iequal(res.get('flags', -1), flags, level + 1) and \
            iequal(res.get('val', val), val, level + 1)

def check(key, flags, val, level = 0):
    res = mc.get(key)
    __check(res[0], flags, val, level + 1)

print("""#---------------------# test protocol boundary overruns #---------------------#""")

for i in range(1900, 2100):
    print ("iteration %d" % i)
    key  = "test_key_%d" % i
    val  = "x" * i
    mc.setq(key, val, flags=82, nosend=True)
    mc.setq("alt_%s" % key, "blah", flags=82, nosend=True)
    data = "".join(mc.commands)
    mc.commands = []

    if (len(data) > 2024):
        for j in range(2024, min(2096, len(data))):
            mc.socket.sendall(data[:j])
            time.sleep(0.00001)
            mc.socket.sendall(data[j:])
    else:
        mc.socket.sendall(data)
    check(key, 82, val)
    check("alt_%s" % key, 82, "blah")

sys.path = saved_path
