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
        return False
    return True

def issert(stmt, level = 1):
    if not bool(stmt):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: result is False" % (tb[0], tb[1])
        return False
    return True

def __check(res, flags, val, level = 0):
    return iequal(res.get('flags', -1), flags, level + 1) and \
            iequal(res.get('val', val), val, level + 1)

def check(key, flags, val, level = 0):
    res = mc.get(key)
    __check(res[0], flags, val, level + 1)

print("""#-----------------------------# test expiration #-----------------------------#""")

stat = mc.stat("reset")

for i in xrange(10000):
    mc.set('key-%d' % i, 'value-%d' % i, expire=1)

stat = mc.stat()
while 'evictions' not in stat or int(stat['evictions']) < 10000:
    time.sleep(0.01)
    stat = mc.stat()

issert('evictions' in stat)
iequal(int(mc.stat().get('evictions', 0)), 10000)

sys.path = saved_path
