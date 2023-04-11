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

print("""# expire: after 1 second""")

server.admin("box.space.__mc_memcached:truncate()", silent=True)

lsn = server.get_lsn(master_id)

while True:
    print("""# set foo""")
    mc_client("set foo 0 1 6\r\nfooval\r\n")

    print("""# foo shoud exists""")
    mc_client("get foo\r\n")
    break

server.wait_lsn(master_id, lsn + 2)

print("""# foo shoud expired""")
mc_client("get foo\r\n")

lsn = server.get_lsn(master_id)

while True:
    print("""# expire: time - 1 second""")
    expire = time.time() - 1

    print("""# set foo""")
    mc_client("set foo 0 %d 6\r\nfooval\r\n" % expire, silent = True)

    print("""# foo shoud be expired""")
    mc_client("get foo\r\n")
    break

server.wait_lsn(master_id, lsn + 2)

lsn = server.get_lsn(master_id)

while True:
    print("""# expire: time + 1 second""")
    expire = time.time() + 1

    print("""# set foo""")
    mc_client("set foo 0 %d 6\r\nfooval\r\n" % expire, silent = True)

    print("""# foo shoud exists""")
    mc_client("get foo\r\n")
    break

server.wait_lsn(master_id, lsn + 2)

print("""# foo shoud be expired""")
mc_client("get foo\r\n")

lsn = server.get_lsn(master_id)

while True:
    print("""# expire: time - 20 second""")
    expire = time.time() - 20

    print("""# set boo""")
    mc_client("set boo 0 %d 6\r\nbooval\r\n" % expire, silent = True)

    print("""# foo shoud expired""")
    mc_client("get boo\r\n")
    break

server.wait_lsn(master_id, lsn + 2)

print("""# expire: after 2 second""")

lsn = server.get_lsn(master_id)

while True:
    print("""# add add""")
    mc_client("add add 0 1 6\r\naddval\r\n")

    print("""# readd add - shoud be fail""")
    mc_client("add add 0 1 7\r\naddval1\r\n")
    break

server.wait_lsn(master_id, lsn + 2)

print("""# readd add - shoud be success""")
mc_client("add add 0 1 7\r\naddval2\r\n")

server.wait_lsn(master_id, lsn + 3)
print("""# return from RO - key should be removed""")
mc_client("add key 0 1 3\r\nfoo\r\n")
print("""# set RO""")
server.admin("box.cfg({read_only = true})", silent=True)
log = server.get_log()
while log.seek_once("Expire: the instance has been moved to a read-only mode") < 0:
    time.sleep(0.01)
time.sleep(1)
print("""# set RW""")
server.admin("box.cfg({read_only = false})", silent=True)
server.wait_lsn(master_id, lsn + 6)
server.admin("box.space.__mc_memcached:len()", silent=False)

mc_client("flush_all\r\n")

sys.path = saved_path
