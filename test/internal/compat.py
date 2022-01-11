import sys

# Added for compatibility with Python 3.

# Useful for very coarse version differentiation.
PY3 = sys.version_info[0] == 3
PY2 = sys.version_info[0] == 2

if PY3:
    string_types = str,
    integer_types = int,
else:
    string_types = basestring,      # noqa: F821
    integer_types = (int, long)     # noqa: F821

def assert_bytes(b):
    """ Ensure given value is <bytes>.
    """
    if type(b) != bytes:
        raise ValueError('Internal error: expected {}, got {}: {}'.format(
            str(bytes), str(type(b)), repr(b)))

def assert_str(s):
    """ Ensure given value is <str>.
    """
    if type(s) != str:
        raise ValueError('Internal error: expected {}, got {}: {}'.format(
            str(str), str(type(s)), repr(s)))

def bytes_to_str(b):
    """ Convert <bytes> to <str>.

        No-op on Python 2.
    """
    assert_bytes(b)
    if PY2:
        return b
    return b.decode('utf-8')

def str_to_bytes(s):
    """ Convert <str> to <bytes>.

        No-op on Python 2.
    """
    assert_str(s)
    if PY2:
        return s
    return s.encode('utf-8')
