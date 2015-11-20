import os
import sys
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
        return False
    return True

def inequal(left, right, level = 0):
    if (left == right):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: %s equal %s" % (tb[0], tb[1],
                repr(left), repr(right))
        return False
    return True

def issert(stmt, level = 0):
    if bool(stmt):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: is False" % (tb[0], tb[1],
                repr(left), repr(right))
        return False
    return True

def __check(res, flags, val, level = 0):
    return iequal(res.get('flags', -1), flags, level + 1) and \
            iequal(res.get('val', val), val, level + 1)

def check(key, flags, val, level = 1):
    res = mc.get(key)
    __check(res[0], flags, val, level + 1)

def set(key, expire, flags, value):
    res = mc.set(key, value, expire, flags)
    return check(key, flags, value, 1)

def empty(key):
    res = mc.get(key)
    return iequal(res[0]['status'], STATUS['KEY_ENOENT'], 2)

def delete(key, when):
    res = mc.delete(key)
    empty(key)


print("""#-----------------------------------------------------------------------------#
################################# Toobig Tests ################################
#-----------------------------------------------------------------------------#""")

# Toobig tests
mc.flush()
empty("toobig")
mc.set("toobig", "not too big", 10, 10)
val = "x" * 1024*1024*2
res = mc.set("toobig", val, 10, 10);
iequal(res[0]['status'], STATUS['E2BIG'])
check("toobig", 10, "not too big")

sys.path = saved_path
