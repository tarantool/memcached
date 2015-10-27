package = 'memcached'
version = 'scm-1'

source  = {
    url    = 'git://github.com/tarantool/memcached.git';
    branch = 'master';
}

description = {
    summary  = "Memcached protocol module for tarantool";
    detailed = [[
    Memcached protocol module for tarantool
    ]];
    homepage = 'https://github.com/tarantool/memcached.git';
    license  = 'BSD';
    maintainer = "Eugine Blikh <bigbes@tarantool.org>";
}

dependencies = {
    'lua >= 5.1';
}

external_dependencies = {
    TARANTOOL = {
        header = 'tarantool/tarantool.h'
    };
}

build = {
    type = 'builtin',

    modules = {
        ['memcached'] = 'memcached/init.lua';
        ['memcached.internal'] = {
            incdirs = {
                '$(TARANTOOL_INCDIR)/tarantool',
                'third_party'
            };
            sources = {
                'memcached/internal/memcached.c',
                'memcached/internal/memcached_constants.c',
                'memcached/internal/memcached_layer.c',
                'memcached/internal/memcached_network.c',
                'memcached/internal/memcached_persistent.c'
            };
        }
    }
}
-- vim: syntax=lua ts=4 sts=4 sw=4 et
