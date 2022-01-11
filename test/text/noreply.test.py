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

print("""# Test that commands can take 'noreply' parameter. """)
mc_client("flush_all noreply\r\n")
mc_client("flush_all 0 noreply\r\n")

mc_client("get noreply:foo\r\n")
mc_client("add noreply:foo 0 0 1 noreply\r\n1\r\n")
mc_client("get noreply:foo\r\n")

mc_client("set noreply:foo 0 0 1 noreply\r\n2\r\n")
mc_client("get noreply:foo\r\n")

mc_client("replace noreply:foo 0 0 1 noreply\r\n3\r\n")
mc_client("get noreply:foo\r\n")

mc_client("append noreply:foo 0 0 1 noreply\r\n4\r\n")
mc_client("get noreply:foo\r\n")

mc_client("prepend noreply:foo 0 0 1 noreply\r\n5\r\n")
mc_client("get noreply:foo\r\n")

sys.stdout.write("gets noreply:foo\r\n")
result = mc_client("gets noreply:foo\r\n", silent = True)
unique_id = int(result.split()[4])

sys.stdout.write("cas noreply:foo 0 0 1 <unique_id> noreply\r\n6\r\n")
mc_client("cas noreply:foo 0 0 1 %d noreply\r\n6\r\n" % unique_id, silent = True)
mc_client("get noreply:foo\r\n")

mc_client("incr noreply:foo 3 noreply\r\n")
mc_client("get noreply:foo\r\n")

mc_client("decr noreply:foo 2 noreply\r\n")
mc_client("get noreply:foo\r\n")

mc_client("delete noreply:foo noreply\r\n")
mc_client("get noreply:foo\r\n")

mc_client("flush_all\r\n")

sys.path = saved_path
