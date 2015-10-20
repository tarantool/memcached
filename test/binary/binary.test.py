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

print("""#---------------------------# flush and noop tests #--------------------------#""")

mc.flush()
iequal(mc.noop()[0]['op'], COMMANDS['noop'][0])

set("x", 5, 19, "somevalue")
delete("x", 0)

set("x", 5, 19, "somevaluex")
set("y", 5, 17, "somevaluey")
mc.flush()
empty("x")
empty("y")

print("""#-----------------------------------------------------------------------------#
################################ Diagnostics ADD ##############################
#-----------------------------------------------------------------------------#""")

empty("i")
mc.add("i", "ex", 10, 5)
check("i", 5, "ex")
res = mc.add("i", "ex2", 10, 5)
iequal(res[0]['status'], STATUS['KEY_EEXISTS'])
check("i", 5, "ex")

print("""#-----------------------------------------------------------------------------#
############################# Diagnostics REPLACE #############################
#-----------------------------------------------------------------------------#""")

# Diagnostics for replace
empty("j")
res = mc.replace("j", "ex", 5, 19)

iequal(res[0]['status'], STATUS['KEY_ENOENT'])
empty("j")
mc.add("j", "ex2", 5, 14)
check("j", 14, "ex2")
mc.replace("j", "ex3", 5, 24)
check("j", 24, "ex3")

print("""#-----------------------------------------------------------------------------#
############################# Diagnostics multiGET ############################
#-----------------------------------------------------------------------------#""")

# Diagnostics "MultiGet"
mc.add("xx", "ex", 5, 1)
mc.add("wye", "why", 5, 2)
mc.getq("xx", nosend=True)
mc.getq("wye", nosend=True)
mc.getq("zed", nosend=True)
res = mc.noop()
__check(res[0], 1, "ex")
__check(res[1], 2, "why")
iequal(len(res), 3)

print("""#-----------------------------------------------------------------------------#
############################ Diagnostics INCR/DECR ############################
#-----------------------------------------------------------------------------#""")

# Test Increment
res = mc.flush()
res = mc.incr("x", 0, expire=0)
iequal(res[0]['val'], 0)
res = mc.incr("x", 1, expire=0)
iequal(res[0]['val'], 1)
res = mc.incr("x", 211, expire=0)
iequal(res[0]['val'], 212)
res = mc.incr("x", 2**33, expire=0)
iequal(res[0]['val'], 8589934804)

print("""#------------------------------# increment error #----------------------------#""")
mc.set("issue48", "text", 0, 0)
res = mc.incr("issue48")
iequal(res[0]['status'], STATUS['DELTA_BADVAL'])
check("issue48", 0, "text")
res = mc.decr("issue48")
iequal(res[0]['status'], STATUS['DELTA_BADVAL'])
check("issue48", 0, "text")
print("""#------------------------------# test decrement #-----------------------------#""")
mc.flush()
res = mc.incr("x", 0, 0, 5)
iequal(res[0]['val'], 5)
res = mc.decr("x")
iequal(res[0]['val'], 4)
res = mc.decr("x", 211)
iequal(res[0]['val'], 0)

print("""#---------------------------------# bug 220 #---------------------------------#""")
res = mc.set("bug220", "100", 0, 0)
ires = mc.incr("bug220", 999)
inequal(res[0]['cas'], ires[0]['cas']) and iequal(ires[0]['val'], 1099)
ires2 = mc.get("bug220")
iequal(ires2[0]['cas'], ires[0]['cas'])
ires = mc.incr("bug220", 999)
inequal(res[0]['cas'], ires[0]['cas']) and iequal(ires[0]['val'], 2098)
ires2 = mc.get("bug220")
iequal(ires2[0]['cas'], ires[0]['cas'])

print("""#----------------------------------# bug 21 #---------------------------------#""")
mc.add("bug21", "9223372036854775807", 0, 0)
res = mc.incr("bug21")
iequal(res[0]['val'], 9223372036854775808)
res = mc.incr("bug21")
iequal(res[0]['val'], 9223372036854775809)
res = mc.decr("bug21")
iequal(res[0]['val'], 9223372036854775808)

print("""#-----------------------------------------------------------------------------#
################################ Diagnostics CAS ##############################
#-----------------------------------------------------------------------------#""")

mc.flush()
res = mc.set("x", "bad value", 5, 19, cas=0x7FFFFFF)
iequal(res[0]['status'], STATUS['KEY_ENOENT'])

res = mc.add("x", "original value", 19, 5)
ires2 = mc.get("x")
iequal(res[0]['cas'], ires2[0]['cas']) and iequal(ires2[0]['val'], 'original value')

res = mc.set("x", "broken value", 5, 19, cas=ires2[0]["cas"] + 1)
iequal(res[0]['status'], STATUS['KEY_EEXISTS'])

res = mc.set("x", "new value", 5, 19, cas=ires2[0]["cas"])
ires = mc.get("x")
iequal(res[0]['cas'], ires[0]['cas']) and iequal(ires[0]['val'], 'new value')

res = mc.set("x", "replay value", 5, 19, cas=ires2[0]["cas"])
iequal(res[0]['status'], STATUS['KEY_EEXISTS'])

def check_empty_response(con):
    res = con.noop()
    return iequal(len(res), 1, 1) and iequal(res[0]['op'], COMMANDS['noop'][0])

print("""#--------------------------------# silent get #-------------------------------#""")
key, val, flags = "silentset", "siltensetval", 82
empty(key)
mc.setq(key, val, flags=flags, expire=0, nosend=True)
check_empty_response(mc)
check(key, flags, val)

print("""#--------------------------------# silent put #-------------------------------#""")
key, val, flags = "silentadd", "siltenaddval", 82
empty(key)
mc.addq(key, val, flags=flags, expire=0, nosend=True)
check_empty_response(mc)
check(key, flags, val)

print("""#------------------------------# silent replace #-----------------------------#""")
key, val, flags = "silentreplace", "somevalue", 829
empty(key)
mc.add(key, "xval", 0, 831)
check(key, 831, "xval")
mc.replaceq(key, val, flags=flags, nosend=True)
check_empty_response(mc)
check(key, flags, val)

print("""#------------------------------# silent delete #------------------------------#""")
key, val, flags = "silentdelete", "someval", 19
empty(key)
mc.set(key, val, flags=flags, expire=0)
check(key, flags, val)
mc.deleteq(key, nosend=True)
empty(key)

print("""#-----------------------------# silent increment #----------------------------#""")
key, opaque = "silentincr", 98428747
empty(key)

mc.incrq(key, 0, 0, 0, nosend=True)
res = mc.incr (key, 0)
iequal(res[0]['val'], 0)

mc.incrq(key, 8, 0, 0, nosend=True)
res = mc.incr (key, 0)
iequal(res[0]['val'], 8)

# Silent decrement
print("""#-----------------------------# silent decrement #----------------------------#""")
key, opaque = "silentdecr", 98428747
empty(key)
mc.decrq(key, 0, 0, 185, nosend=True)
res = mc.decr (key, 0)
iequal(res[0]['val'], 185)
mc.decrq(key, 8, 0, 0, nosend=True)
res = mc.decr (key, 0)
iequal(res[0]['val'], 177)

print("""#-------------------------------# silent flush #------------------------------#""")
stat1 = mc.stat()
set("x", 5, 19, "somevaluex")
set("y", 5, 19, "somevaluey")
mc.flushq(nosend=True)
empty("x")
empty("y")

stat2 = mc.stat()
iequal(int(stat1['cmd_flush']) + 1, int(stat2['cmd_flush']))

print("""#----------------------------# diagnostics append #---------------------------#""")
key, value, suffix = "appendkey", "some value", " more"
set(key, 8, 19, value)
mc.append(key, suffix)
check(key, 19, value + suffix)

print("""#---------------------------# diagnostics prepend #---------------------------#""")
key, value, prefix = "prependkey", "some value", "more "
set(key, 8, 19, value)
mc.prepend(key, prefix)
check(key, 19, prefix + value)

print("""#------------------------------# silent append #------------------------------#""")
key, value, suffix = "appendqkey", "some value", " more"
set(key, 8, 19, value)
mc.appendq(key, suffix, nosend=True)
check_empty_response(mc)
check(key, 19, value + suffix)

print("""#------------------------------# silent prepend #-----------------------------#""")
key, value, prefix = "prependqkey", "some value", "more "
set(key, 8, 19, value)
mc.prependq(key, prefix, nosend=True)
check_empty_response(mc)
check(key, 19, prefix + value)

sys.path = saved_path
