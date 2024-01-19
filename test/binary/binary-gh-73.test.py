import os
import sys
import inspect

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedBinaryConnection
from internal.memcached_connection import STATUS, COMMANDS, CONNECT_PORT

mc = MemcachedBinaryConnection("127.0.0.1", CONNECT_PORT)

batch_count = 20 # Default batch_count value used in memcached.

# Put (batch_count + 1) keys at once, pay attention to "nosend" flag.
for i in range(1, batch_count + 1):
    val = "test_val_%d" % i
    key = "test_key_%d" % i
    mc.setq(key, val, flags = 82, nosend=True)

data = b"".join(mc.commands)
mc.socket.sendall(data)

# Get any key right after setting.
res = mc.get("test_key_20")
print(res[0]['val'])

sys.path = saved_path
