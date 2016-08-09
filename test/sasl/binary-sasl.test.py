import os
import sys
import inspect
import traceback

saved_path = sys.path[:]
sys.path.append(os.path.dirname(os.path.abspath(inspect.getsourcefile(lambda:0))))

from internal.memcached_connection import MemcachedBinaryConnection
from internal.memcached_connection import STATUS, COMMANDS

mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)

def iequal(left, right, level = 1):
    if (left != right):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: %s not equal %s" % (tb[0], tb[1],
                repr(left), repr(right))
        return False
    return True

def inequal(left, right, level = 0):
    if (left == right):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: %s equal %s" % (tb[0], tb[1],
                repr(left), repr(right))
        return False
    return True

def issert(stmt, level = 0):
    if bool(stmt):
        tb = traceback.extract_stack()[-(level + 1)]
        print "Error on line %s:%d: is False" % (tb[0], tb[1],
                repr(left), repr(right))
        return False
    return True

def __check(res, flags, val, level = 0):
    return iequal(res.get('flags', -1), flags, level + 1) and \
            iequal(res.get('val', val), val, level + 1)

def check(key, flags, val, level = 0):
    res = mc.get(key)
    __check(res[0], flags, val, level + 1)

def set(key, expire, flags, value):
    res = mc.set(key, value, expire, flags)
    return check(key, flags, value, 1)

def empty(key):
    res = mc.get(key)
    return iequal(res[0]['status'], STATUS['KEY_ENOENT'], 1)

def delete(key, when):
    res = mc.delete(key)
    empty(key)

print("""#---------------------------# sasl tests #--------------------------#""")

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    res = mc1.set('x', 'somevalue')
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    res = mc1.delete('x', 'somevalue')
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    res = mc1.set('x', 'somevalue')
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    res = mc1.flush()
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

res = mc.sasl_list()
if res[0]['val'].find('PLAIN') == -1:
    print "Error"

secret = '%c%s%c%s' % (0, 'testuser', 0, 'testpass')
res = mc.sasl_start("X" * 40, secret)
iequal(res[0]['status'], STATUS['AUTH_ERROR'])
# mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)

secret = '%c%s%c%s' % (0, 'testuser', 0, 'badpass')
res = mc.sasl_start("PLAIN", secret)
iequal(res[0]['status'], STATUS['AUTH_ERROR'])
# mc = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)

secret = '%c%s%c%s' % (0, 'testuser', 0, 'testpass')
res = mc.sasl_start("PLAIN", secret)
iequal(res[0]['status'], 0)

if True:
    empty('x')
    res = mc.set('x', 'somevalue')
    iequal(res[0]['status'], 0)

if True:
    check('x', 0, 'somevalue')
    res = mc.delete('x', 'somevalue')
    iequal(res[0]['status'], 0)

if True:
    empty('x')
    res = mc.set('x', 'somevalue')
    iequal(res[0]['status'], 0)

if True:
    check('x', 0, 'somevalue')
    mc.flush()
    empty('x')

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    secret = '%c%s%c%s' % (0, 'testuser', 0, 'wrongpass')
    res = mc1.sasl_start('PLAIN', secret)
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

    res = mc1.set('x', 'somevalue')
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

if True:
    mc1 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    secret = '%c%s%c%s' % (0, 'testuser', 0, 'wrongpass')
    res = mc1.sasl_start('PLAIN', secret)
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

    mc2 = MemcachedBinaryConnection("127.0.0.1", iproto.py_con.port)
    secret = '%c%s%c%s' % (0, 'testuser', 0, 'testpass')
    res = mc2.sasl_start('PLAIN', secret)
    iequal(res[0]['status'], 0)
    res = mc2.set('x', 'somevalue')
    iequal(res[0]['status'], 0)

    res = mc1.set('x', 'somevalue')
    iequal(res[0]['status'], STATUS['AUTH_ERROR'])

if True:
    res = mc.stat()
    iequal(res['auth_cmds'], '6')
    iequal(res['auth_errors'], '4')

sys.path = saved_path
