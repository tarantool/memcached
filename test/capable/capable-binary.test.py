import shlex

from subprocess import PIPE, Popen, STDOUT

cmd = shlex.split('capable/memcapable -b -p %s -h %s -v' %
        (iproto.py_con.port, '127.0.0.1'))

task = Popen(cmd, stdout=PIPE, stderr=STDOUT)

print task.communicate()[0]
