import os
import sys
import time
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedTextConnection

port = int(iproto.uri.split(':')[1]) + 3
mc_client = MemcachedTextConnection('localhost', port)

blobs_list = [ "mooo\0", "mumble\0\0\0\0\r\rblarg", "\0", "\r" ]

for i in range(len(blobs_list)):
    key = "foo_%d" % i
    blob = blobs_list[i]
    blob_len = len(blob)

    mc_client("set %s 0 0 %d\r\n%s\r\n" % (key, blob_len, blob))
    mc_client("get %s\r\n" % key)

sys.path = saved_path
