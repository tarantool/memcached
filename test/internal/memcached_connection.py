import re
import sys

from struct import Struct

from lib.tarantool_connection import TarantoolConnection

HEADER        = '!BBHBBHLLQ'
HEADER_STRUCT = Struct(HEADER)
HEADER_SIZE   = 24

LENGTH        = "!8xL"
LENGTH_STRUCT = Struct(LENGTH)

INDECR        = "!Q"
INDECR_STRUCT = Struct(INDECR)

MAGIC = {
    'request' : 0x80,
    'response': 0x81
}

COMMANDS = {
    'get'     : [0x00, Struct(HEADER + ''),    0,  None, 0],
    'set'     : [0x01, Struct(HEADER + 'LL'),  8,  None, None],
    'add'     : [0x02, Struct(HEADER + 'LL'),  8,  None, None],
    'replace' : [0x03, Struct(HEADER + 'LL'),  8,  None, None],
    'delete'  : [0x04, Struct(HEADER + ''),    0,  None, 0],
    'incr'    : [0x05, Struct(HEADER + 'QQL'), 20, None, 0],
    'decr'    : [0x06, Struct(HEADER + 'QQL'), 20, None, 0],
    'quit'    : [0x07, Struct(HEADER + ''),    0,  0,    0],
    'flush'   : [0x08, Struct(HEADER + 'I'),   4,  0,    0],
    'getq'    : [0x09, Struct(HEADER + ''),    0,  None, 0],
    'noop'    : [0x0a, Struct(HEADER + ''),    0,  0,    0],
    'version' : [0x0b, Struct(HEADER + ''),    0,  0,    0],
    'getk'    : [0x0c, Struct(HEADER + ''),    0,  None, 0],
    'getkq'   : [0x0d, Struct(HEADER + ''),    0,  None, 0],
    'append'  : [0x0e, Struct(HEADER + ''),    0,  None, None],
    'prepend' : [0x0f, Struct(HEADER + ''),    0,  None, None],
    'stat'    : [0x10, Struct(HEADER + ''),    0,  None, 0],
    'setq'    : [0x11, Struct(HEADER + 'LL'),  8,  None, None],
    'addq'    : [0x12, Struct(HEADER + 'LL'),  8,  None, None],
    'replaceq': [0x13, Struct(HEADER + 'LL'),  8,  None, None],
    'deleteq' : [0x14, Struct(HEADER + ''),    0,  None, 0],
    'incrq'   : [0x15, Struct(HEADER + 'QQL'), 20, None, 0],
    'decrq'   : [0x16, Struct(HEADER + 'QQL'), 20, None, 0],
    'quitq'   : [0x17, Struct(HEADER + ''),    0,  0,    0],
    'flushq'  : [0x18, Struct(HEADER + 'I'),   4,  0,    0],
    'appendq' : [0x19, Struct(HEADER + ''),    0,  None, None],
    'prependq': [0x1A, Struct(HEADER + ''),    0,  None, None],
    'touch'   : [0x1C, Struct(HEADER + 'L'),   4,  None, 0],
    'gat'     : [0x1D, Struct(HEADER + 'L'),   4,  None, 0],
    'gatq'    : [0x1E, Struct(HEADER + 'L'),   4,  None, 0],
    'gatk'    : [0x23, Struct(HEADER + 'L'),   4,  None, 0],
    'gatkq'   : [0x24, Struct(HEADER + 'L'),   4,  None, 0],
}

def is_indecrq(cmd):
    if cmd in (COMMANDS['incr'][0], COMMANDS['decr'][0], COMMANDS['incrq'][0],
            COMMANDS['decrq'][0]):
        return True
    return False

ANSWERS = {
    0x00: [Struct('!L'), 4, 0,    None], # get
    0x01: [None,         0, 0,    0],    # set
    0x02: [None,         0, 0,    0],    # add
    0x03: [None,         0, 0,    0],    # replace
    0x04: [None,         0, 0,    0],    # delete
    0x05: [None,         0, 0,    8],    # incr
    0x06: [None,         0, 0,    8],    # decr
    0x07: [None,         0, 0,    0],    # quit
    0x08: [None,         0, 0,    0],    # flush
    0x09: [Struct('!L'), 4, 0,    None], # getq
    0x0a: [None,         0, 0,    0],    # noop
    0x0b: [None,         0, 0,    None], # version
    0x0c: [Struct('!L'), 4, None, None], # getk
    0x0d: [Struct('!L'), 4, None, None], # getkq
    0x0e: [None,         0, 0,    0],    # append
    0x0f: [None,         0, 0,    0],    # prepend
    0x10: [None,         0, None, None], # stat
    0x11: [None,         0, 0,    0],    # setq
    0x12: [None,         0, 0,    0],    # addq
    0x13: [None,         0, 0,    0],    # replaceq
    0x14: [None,         0, 0,    0],    # deleteq
    0x15: [None,         0, 0,    8],    # incrq
    0x16: [None,         0, 0,    8],    # decrq
    0x17: [None,         0, 0,    0],    # quitq
    0x18: [None,         0, 0,    0],    # flushq
    0x19: [None,         0, 0,    0],    # appendq
    0x1A: [None,         0, 0,    0],    # prependq
    0x1C: [None,         0, 0,    0],    # touch
    0x1D: [Struct('!L'), 4, 0,    None], # gat
    0x1E: [Struct('!L'), 4, 0,    None], # gatq
    0x23: [Struct('!L'), 4, None, None], # gatk
    0x24: [Struct('!L'), 4, None, None], # gatkq
}

STATUS = {
    'OK'              : 0x00,
    'KEY_ENOENT'      : 0x01,
    'KEY_EEXISTS'     : 0x02,
    'E2BIG'           : 0x03,
    'EINVAL'          : 0x04,
    'NOT_STORED'      : 0x05,
    'DELTA_BADVAL'    : 0x06,
    'VBUCKET_BADVAL'  : 0x07,
    'AUTH_ERROR'      : 0x20,
    'AUTH_CONTINUE'   : 0x21,
    'UNKNOWN_COMMAND' : 0x81,
    'ENOMEM'          : 0x82,
    'NOT_SUPPORTED'   : 0x83,
    'SERVER_ERROR'    : 0x84,
    'EBUSY'           : 0x85,
    'EAGAIN'          : 0x86,
}

TYPE = {
    'raw' : 0x00,
}

class MemcachedException(Exception):
    def __init__(self, msg, size):
        self.msg  = msg
        self.size = size

def construct_query(cmd, args):
    assert(cmd in COMMANDS)
    op, struct, ext_len, key_len, val_len = COMMANDS[cmd]
    key     = args.get('key', '')
    val     = args.get('val', '')
    key_len = 0 if key_len is not None else len(key)
    val_len = 0 if val_len is not None else len(val)
    tot_len = val_len + key_len + ext_len
    dtype   = args.get('type', TYPE['raw'])
    opaque  = args.get('opaque', 0)
    cas     = args.get('cas', 0)
    extra   = args.get('extra', [])
    retval = [
        struct.pack(MAGIC['request'], op, key_len,
                    ext_len, dtype, 0, tot_len,
                    opaque, cas, *extra),
        key, val
    ]
    return ''.join(retval)

def parse_query(cmd):
    to_read = HEADER_SIZE
    if len(cmd) < to_read:
        raise MemcachedException("Need more bytes", to_read - len(cmd))

    a = (magic, op, key_len, ext_len, dtype,
     status, tot_len, opaque, cas) = HEADER_STRUCT.unpack_from(cmd)
    to_read += tot_len
    val_len = tot_len - key_len - ext_len

    if len(cmd) < to_read:
        raise MemcachedException("Need more bytes", to_read - len(cmd))
    struct, ext_lenv, key_lenv, val_lenv = ANSWERS[op]

    # Multiple checks to be confident in server responses
    assert(magic == MAGIC['response'])
    if status == STATUS['OK']:
        assert(ext_len == ext_lenv)
        assert(((key_lenv > 0 or key_lenv is None) and key_len > 0) or key_len == 0)
        assert(((val_lenv > 0 or val_lenv is None) and val_len > 0) or val_len == 0)
    else:
        assert(val_len > 0)

    retval = {
        'magic'  : magic,
        'op'     : op,
        'dtype'  : dtype,
        'status' : status,
        'opaque' : opaque,
        'cas'    : cas,
    }
    extra = None
    if struct is not None and status == STATUS['OK']:
        extra = struct.unpack_from(cmd, HEADER_SIZE)
    if extra is not None:
        retval['flags'] = extra[0]

    key = None
    if key_lenv is None:
        begin = HEADER_SIZE + ext_len
        end   = begin + key_len
        key   = cmd[begin:end]
    if key is not None:
        retval['key'] = key

    val = None
    if val_lenv is None or val_len > 0:
        begin = HEADER_SIZE + ext_len + key_len
        end   = HEADER_SIZE + tot_len
        val   = cmd[begin:end]
        # decode result of (incr/decr)(q)
        if is_indecrq(op):
            val = INDECR_STRUCT.unpack_from(val)[0]
    if val is not None:
        retval['val'] = val

    return retval

class MemcachedBinaryConnection(TarantoolConnection):
    def __init__(self, host, port):
        super(MemcachedBinaryConnection, self).__init__(host, port)
        self.latest = 0
        self.opaque = 1
        self.recv_buffer = ''
        self.cas = {}
        self.commands = []
        self.connect()

    def send(self):
        commands = ''.join(self.commands)
        self.socket.sendall(commands)
        self.commands = []

    def _recv(self, to_read):
        buf = b""
        while to_read > 0:
            tmp = self.socket.recv(to_read)
            to_read -= len(tmp)
            buf += tmp
        return buf

    def _read_response(self):
        hdr = self._recv(HEADER_SIZE)
        sz = hdr + self._recv(LENGTH_STRUCT.unpack_from(hdr)[0])
        return sz

    def _read_and_parse_response(self):
        return parse_query(self._read_response())

    def read_responses(self):
        resp = []
        while True:
            obj = self._read_and_parse_response()
            resp.append(obj)
            if obj['cas'] > 0 and 'key' in obj and obj['key']:
                self.cas[obj['key']] = obj['cas']
            if obj['opaque'] == self.latest:
                 break
        return resp

    def get(self, key, nosend=False):
        self.commands.append(construct_query('get', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def getq(self, key, nosend=False):
        self.commands.append(construct_query('getq', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def getk(self, key, nosend=False):
        self.commands.append(construct_query('getk', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def getkq(self, key, nosend=False):
        self.commands.append(construct_query('getkq', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def set(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('set', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def setq(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('setq', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def add(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('add', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def addq(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('addq', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def replace(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('replace', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def replaceq(self, key, value, expire=0, flags=0, nosend=False, cas=-1):
        if (cas == -1):
            cas = self.cas.get(key, 0)
        self.commands.append(construct_query('replaceq', {
            'key': key,
            'cas': cas,
            'val': value,
            'opaque': self.opaque,
            'extra': [flags, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def delete(self, key, nosend=False):
        self.commands.append(construct_query('delete', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def deleteq(self, key, nosend=False):
        self.commands.append(construct_query('deleteq', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def incr(self, key, delta=1, expire=0xFFFFFFFF, initial=0, nosend=False):
        self.commands.append(construct_query('incr', {
            'key': key,
            'opaque': self.opaque,
            'extra': [delta, initial, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def decr(self, key, delta=1, expire=0xFFFFFFFF, initial=0, nosend=False):
        self.commands.append(construct_query('decr', {
            'key': key,
            'opaque': self.opaque,
            'extra': [delta, initial, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def incrq(self, key, delta=1, expire=0xFFFFFFFF, initial=0, nosend=False):
        self.commands.append(construct_query('incrq', {
            'key': key,
            'opaque': self.opaque,
            'extra': [delta, initial, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def decrq(self, key, delta=1, expire=0xFFFFFFFF, initial=0, nosend=False):
        self.commands.append(construct_query('decrq', {
            'key': key,
            'opaque': self.opaque,
            'extra': [delta, initial, expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def quit(self, nosend=False):
        self.commands.append(construct_query('quit', {
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def quitq(self, nosend=False):
        self.commands.append(construct_query('quitq', {
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def flush(self, expire=0, nosend=False):
        self.commands.append(construct_query('flush', {
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def flushq(self, expire=0, nosend=False):
        self.commands.append(construct_query('flushq', {
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def noop(self):
        self.commands.append(construct_query('noop', {
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        self.send()
        return self.read_responses()

    def version(self, nosend=False):
        self.commands.append(construct_query('version', {
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def append(self, key, value, nosend=False):
        self.commands.append(construct_query('append', {
            'key': key,
            'val': value,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def prepend(self, key, value, nosend=False):
        self.commands.append(construct_query('prepend', {
            'key': key,
            'val': value,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def appendq(self, key, value, nosend=False):
        self.commands.append(construct_query('appendq', {
            'key': key,
            'val': value,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def prependq(self, key, value, nosend=False):
        self.commands.append(construct_query('prependq', {
            'key': key,
            'val': value,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def stat(self, key=''):
        self.commands.append(construct_query('stat', {
            'key': key,
            'opaque': self.opaque,
        }))
        self.latest  = self.opaque
        self.opaque += 1
        ans = {}
        self.send()
        while True:
            rv = self._read_and_parse_response()
            if 'key' in rv and not rv['key'] and \
               'val' in rv and not rv['val']:
                return ans
            ans[rv['key']] = rv['val']

    def touch(self, key, expire, nosend=False):
        self.commands.append(construct_query('touch', {
            'key': key,
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def gat(self, key, expire, nosend=False):
        self.commands.append(construct_query('gat', {
            'key': key,
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def gatk(self, key, expire, nosend=False):
        self.commands.append(construct_query('gatk', {
            'key': key,
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def gatq(self, key, expire, nosend=False):
        self.commands.append(construct_query('gat', {
            'key': key,
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

    def gatkq(self, key, expire, nosend=False):
        self.commands.append(construct_query('gatk', {
            'key': key,
            'opaque': self.opaque,
            'extra': [expire]
        }))
        self.latest  = self.opaque
        self.opaque += 1
        if not nosend:
            self.send()
            return self.read_responses()
        return None

MEMCACHED_SEPARATOR = '\r\n'

class MemcachedCommandBuffer(object):
    def __init__(self, commands):
        self.buf = commands

    def read_line(self):
        if self.buf == None:
            return None

        index = self.buf.find(MEMCACHED_SEPARATOR)
        if index > 0:
            line = self.buf[:index]
            self.buf = self.buf[index + 2:]
        else:
            line = self.buf
            self.buf = None
        return line

class MemcachedTextConnection(TarantoolConnection):

    def execute_no_reconnect(self, commands, silent = False):
        self.send(commands, silent)
        return self.recv(silent)

    def send(self, commands, silent = False):
        self.commands = commands
        self.socket.sendall(commands)
        if not silent:
            print "<<" + '-' * 50
            sys.stdout.write(self.commands.strip() + '\n')
            #sys.stdout.write(self.commands)

    def recv(self, silent = False):
        self.recv_buffer = ''
        self.command_buffer = MemcachedCommandBuffer(self.commands)
        self.raw_reply = ''
        self.reply     = ''

        while True:
            cmd = self.command_buffer.read_line()
            if cmd == None:
                # end of buffer reached
                break

            if re.match('set|add|replace|append|prepend|cas', cmd, re.I):
                self.reply_storage(cmd)
            elif re.match('get|gets', cmd, re.I):
                self.reply_retrieval(cmd)
            elif re.match('delete', cmd, re.I):
                self.reply_deletion(cmd)
            elif re.match('incr|decr', cmd, re.I):
                self.reply_incr_decr(cmd)
            elif re.match('stats', cmd, re.I):
                self.reply_stats(cmd)
            elif re.match('flush_all|version|quit', cmd, re.I):
                self.reply_other(cmd)
            elif cmd == '':
                continue
            else:
                self.reply_unknown(cmd)

        if not silent:
            print ">>" + '-'*50
            sys.stdout.write(self.reply.strip() + '\n')
            #sys.stdout.write(self.reply)

        return self.raw_reply

    def reply_storage(self, cmd):
        self.command_buffer.read_line()
        self.reply_single_line(cmd)

    def reply_retrieval(self, cmd):
        while True:
            # read reply cmd
            key = self.read_line()
            self.raw_reply += key + MEMCACHED_SEPARATOR
            # delete cas
            if (re.match('VALUE', key)):
                arr = key.split(' ')
                if (len(arr) > 4):
                    arr[4] = 'cas'
                    key = ' '.join(arr)

            # store line in reply buffer
            self.reply += key + MEMCACHED_SEPARATOR

            # chec reply type
            if re.match('VALUE', key):
                # Value header received
                key_params = key.split()
                if len(key_params) < 4:
                    continue

                # receive value
                value_len = int(key_params[3])
                while value_len > 0:
                    # Receive value line
                    value = self.read_line()
                    # store value line in reply buffer
                    self.raw_reply += value + MEMCACHED_SEPARATOR
                    self.reply += value + MEMCACHED_SEPARATOR
                    # decrease value len
                    value_len -= len(value)
            elif re.match('END', key):
                break
            elif re.match('ERROR|CLIENT_ERROR|SERVER_ERROR', key):
                break
            else:
                # unknown
                print "error: unknown line: '%s'" % key
                self.reply += "error: unknown line: '%s'" % key
                break

    def reply_deletion(self, cmd):
        self.reply_single_line(cmd)

    def reply_incr_decr(self, cmd):
        self.reply_single_line(cmd)

    def reply_stats(self, cmd):
        while True:
            # read reply stats
            stat = self.read_line()
            # store stat in reply buffer
            self.raw_reply += stat + MEMCACHED_SEPARATOR

            a = stat.split(' ')
            if len(a) > 2 and a[1] in ['pid', 'time', 'version', 'pointer_size']:
                self.reply += ' '.join(a[:2]) + ' <var>' + MEMCACHED_SEPARATOR
            else:
                self.reply += stat + MEMCACHED_SEPARATOR

            if re.match('END', stat):
                break
            if re.match('ERROR|CLIENT_ERROR|SERVER_ERROR', stat):
                break

    def reply_other(self, cmd):
        self.reply_single_line(cmd)

    def reply_single_line(self, cmd):
        params = cmd.split()
        if re.match('noreply', params[-1], re.I):
            # Noreply option exist
            noreply = True
        else:
            noreply = False

        if not noreply:
            reply = self.read_line() + MEMCACHED_SEPARATOR
            self.reply     += reply
            self.raw_reply += reply

    def reply_unknown(self, line):
        reply = self.read_line() + MEMCACHED_SEPARATOR
        self.raw_reply += reply
        self.reply += reply

    def read_line(self):
        buf = self.recv_buffer
        while True:
            # try to find separator in the exist buffer
            index = buf.find(MEMCACHED_SEPARATOR)
            if index > 0:
                break
            data = self.socket.recv(1048576)
            if not data:
                return None
            buf += data
        # get line
        line = buf[:index]
        # cut line from receive buffer
        self.recv_buffer = buf[index + 2:]
        return line

