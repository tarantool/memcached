import os
import sys
import shlex
import inspect
import traceback

from subprocess import PIPE, Popen, STDOUT

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedBinaryConnection
from internal.memcached_connection import STATUS, COMMANDS

mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
mc.flush()

cmd = shlex.split('capable/memcapable -a -p %s -h %s -v' %
        (iproto.py_con.port, '127.0.0.1'))

task = Popen(cmd, stdout=PIPE, stderr=STDOUT)

testcase = task.communicate()[0]
print(testcase)

sys.path = saved_path
