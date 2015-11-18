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

master_id = server.get_param('server')['id']

def wait():
    lsn = server.get_lsn(master_id)
    return server.wait_lsn(master_id, lsn+1)

print """# expire: after 1 second"""

print """# set foo"""
mc_client("set foo 0 1 6\r\nfooval\r\n")

print """# foo shoud be exists"""
mc_client("get foo\r\n")

wait()
print """# foo shoud expired"""
mc_client("get foo\r\n")


print """# expire: time - 1 second"""
expire = time.time() - 1

print """# set foo"""
mc_client("set foo 0 %d 6\r\nfooval\r\n" % expire, silent = True)

print """# foo shoud expired"""
mc_client("get foo\r\n")


print """# expire: time + 1 second"""
expire = time.time() + 1

print """# set foo"""
mc_client("set foo 0 %d 6\r\nfooval\r\n" % expire, silent = True)

print """# foo shoud be exists"""
mc_client("get foo\r\n")

wait()
print """# foo shoud expired"""
mc_client("get foo\r\n")

print """# expire: time - 20 second"""
expire = time.time() - 20

print """# set boo"""
mc_client("set boo 0 %d 6\r\nbooval\r\n" % expire, silent = True)

print """# foo shoud expired"""
mc_client("get boo\r\n")

print """# expire: after 2 second"""

print """# add add"""
mc_client("add add 0 1 6\r\naddval\r\n")

print """# readd add - shoud be fail"""
mc_client("add add 0 1 7\r\naddval1\r\n")

wait()
print """# readd add - shoud be success"""
mc_client("add add 0 1 7\r\naddval2\r\n")

sys.path = saved_path
