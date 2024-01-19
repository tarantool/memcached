import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedTextConnection, CONNECT_PORT

mc_client = MemcachedTextConnection('localhost', CONNECT_PORT)

mc_client("boguscommand slkdsldkfjsd\r\n")

sys.path = saved_path
