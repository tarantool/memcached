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
    SMALL = {
        header = 'small/slab_cache.h'
    };
}

build = {
    type = 'builtin',

    modules = {
        ['memcached'] = 'memcached/init.lua';
        ['memcached.internal'] = {
            incdirs = {
                '$(TARANTOOL_INCDIR)/tarantool',
                '$(SMALL_INCDIR)/small',
                'third_party'
            },
            libraries = {
                'small'
            },
            sources = {
                "memcached/internal/constants.c"
                "memcached/internal/utils.c"
                "memcached/internal/proto_binary.c"
                "memcached/internal/proto_text_parser.c"
                "memcached/internal/proto_text.c"
                "memcached/internal/network.c"
                "memcached/internal/memcached_layer.c"
                "memcached/internal/expiration.c"
                "memcached/internal/memcached.c"
            };
        }
    }
}
-- vim: syntax=lua ts=4 sts=4 sw=4 et
