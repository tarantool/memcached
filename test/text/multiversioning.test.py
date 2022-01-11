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

buf_size = 256 * 1024
buf = "0123456789abcdef" * int(buf_size / 16)
buf_upper = buf.upper()

memcached1 = mc_client
memcached2 = MemcachedTextConnection('localhost', port)

print("""# Store big in lower case via first memcached client """)
print("set big 0 0 %d\r\n<big-value-lower-case>" % buf_size)
print(memcached1("set big 0 0 %d\r\n%s\r\n" % (buf_size, buf), silent = True))

print("""# send command 'get big' to first memcached client """)
memcached1.send("get big\r\n")

time.sleep(0.01)
print("""# send command 'delete big' to second client """)
memcached2("delete big\r\n")

print("""# Store big in upper case via second memcached client """)
print("set big 0 0 %d\r\n<big-value-upper-case>" % buf_size)
print(memcached2("set big 0 0 %d\r\n%s\r\n" % (buf_size, buf_upper), silent = True))

print("""# recv reply 'get big' from the first memcached client """)
reply = memcached1.recv(silent = True)
reply_buf = reply.split('\r\n')[1]
if buf == reply_buf:
    print("success: buf == reply")
else:
    print("fail: buf != reply")
    print(len(buf), len(reply_buf))

server.stop()
server.cleanup()
server.start()

mc_client('stats\r\n')

sys.path = saved_path
