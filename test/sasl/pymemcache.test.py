import os
import sys
import inspect
import traceback

from pymemcache.client.base import Client

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedBinaryConnection
from internal.memcached_connection import STATUS, COMMANDS


client = Client(('localhost', iproto.py_con.port))
try:
    print client.set('some_key', 'some_value')
except MemcacheUnexpectedCloseError:
    print 'Connection closed'

# Check tarantool
mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
secret = '%c%s%c%s' % (0, 'testuser', 0, 'testpass')
res = mc.sasl_start("PLAIN", secret)
print 'status: ', res[0]['status']

sys.path = saved_path
