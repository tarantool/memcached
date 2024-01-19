import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedTextConnection, CONNECT_PORT

mc_client = MemcachedTextConnection('localhost', CONNECT_PORT)

mc_client("flush_all\r\n", silent = True)

print("""# set foo (and should get it) """)
mc_client("set foo 0 0 6\r\nfooval\r\n")
mc_client("get foo\r\n")

print("""# add bar (and should get it)""")
mc_client("set bar 0 0 6\r\nbarval\r\n")
mc_client("get bar\r\n")

print("""# add foo (but shouldn't get new value)""")
mc_client("add foo 0 0 5\r\nfoov2\r\n")
mc_client("get foo\r\n")

print("""# replace bar (should work)""")
mc_client("replace bar 0 0 6\r\nbarva2\r\n")
mc_client("get bar\r\n")

print("""# replace notexist (shouldn't work)""")
mc_client("replace notexist 0 0 6\r\nbarva2\r\n")
mc_client("get notexist\r\n")

print("""# delete foo""")
mc_client("delete foo\r\n")
mc_client("get foo\r\n")

print("""# delete foo again. not found this time.""")
mc_client("delete foo\r\n")
mc_client("get foo\r\n")

print("""# add moo""")
mc_client("add moo 0 0 6\r\nmooval\r\n")
mc_client("get moo\r\n")

print("""# check-and-set (cas) failure case, try to set value with incorrect cas unique val""")
mc_client("cas moo 0 0 6 0\r\nMOOVAL\r\n")
mc_client("get moo\r\n")

result = mc_client("gets moo\r\n", silent = True)
unique_id = int(result.split()[4])

print("""# now test that we can store moo with the correct unique id""")
mc_client("cas moo 0 0 6 %d\r\nMOOVAL\r\n" % unique_id, silent = True)
mc_client("get moo\r\n")

mc_client("set foo 0 0 6\r\nfooval\r\ndelete foo\r\nset foo 0 0 6\r\nfooval\r\ndelete foo\r\n")

len = 1024
while len < (1024 * 1028):
    val = 'B' * len
    if len > (1024 * 1024):
        print("""# Ensure causing a memory overflow doesn't leave stale data.""")
        print("# set small data: - should pass")
        mc_client("set foo_%d 0 0 3\r\nMOO\r\n" % (len))
        mc_client("get foo_%d\r\n" % (len))
        print("# set big data: - should fail")
        print("set foo_%d 0 0 %d\r\n<big-data>\r\n" % (len, len))
        print(mc_client("set foo_%d 0 0 %d\r\n%s\r\n" % (len, len, val), silent = True))
    else:
        print("# set big data: - should pass")
        print("set foo_%d 0 0 %d\r\n<big-data>\r\n" % (len, len))
        print(mc_client("set foo_%d 0 0 %d\r\n%s\r\n" % (len, len, val), silent = True))
    len += 1024 * 512

print("""#
# A test for Bug#898198 memcached protocol isn't case-insensitive"
#""")

mc_client("SET foo 0 0 6\r\nfooval\r\n")
mc_client("GET foo\r\n")
mc_client("ADD foo 0 0 5\r\nfoov2\r\n")
mc_client("GET foo\r\n")
mc_client("REPLACE bar 0 0 6\r\nbarva2\r\n")
mc_client("GET bar\r\n")
mc_client("DELETE foo\r\n")
mc_client("GET foo\r\n")
mc_client("CAS moo 0 0 6 0\r\nMOOVAL\r\n")
mc_client("GET moo\r\n")
mc_client("GETS moo\r\n")

mc_client("flush_all\r\n")

sys.path = saved_path
