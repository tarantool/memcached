import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedTextConnection, CONNECT_PORT

mc_client = MemcachedTextConnection('localhost', CONNECT_PORT)

flags_list = [0x0, 0x7b, 0xffff]

for flags in flags_list:
    mc_client("set foo %d 0 6\r\nfooval\r\n" % flags)
    result = mc_client("gets foo\r\n")
    ret_flags = int(result.split()[2])
    if flags == ret_flags:
        print("success: flags (0x%x) == ret_flags (0x%x)" % (flags, ret_flags))
    else:
        print("fail: flags (0x%x) != ret_flags (0x%x)" % (flags, ret_flags))

mc_client("flush_all\r\n")

sys.path = saved_path
