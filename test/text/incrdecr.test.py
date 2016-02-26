import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedTextConnection

port = int(iproto.uri.split(':')[1])
mc_client = MemcachedTextConnection('localhost', port)

print """# incr/decr big value """
mc_client("set bug21 0 0 19\r\n9223372036854775807\r\n")
mc_client("incr bug21 1\r\n")
mc_client("incr bug21 1\r\n")
mc_client("decr bug21 1\r\n")

print """# underflow protection """
mc_client("set num 0 0 1\r\n1\r\n")
mc_client("incr num 1\r\n")
mc_client("incr num 8\r\n")
mc_client("decr num 1\r\n")
mc_client("decr num 9\r\n")
mc_client("decr num 5\r\n")

print """# 32-bit value """
mc_client("set num 0 0 10\r\n4294967296\r\n")
mc_client("incr num 1\r\n")

print """# overflow value """
mc_client("set num 0 0 20\r\n18446744073709551615\r\n")
mc_client("incr num 1\r\n")

print """# bogus """
mc_client("decr bogus 1\r\n")
mc_client("decr incr 1\r\n")

print """# bit increment """
mc_client("set bigincr 0 0 1\r\n0\r\n")
mc_client("incr bigincr 18446744073709551610\r\n")

print """# incr text value error """
mc_client("set text 0 0 2\r\nhi\r\n")
mc_client("incr text 1\r\n")

mc_client("flush_all\r\n")

sys.path = saved_path
