package = 'memcached'
version = 'scm-1'

source  = {
    url    = 'git+https://github.com/tarantool/memcached.git';
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

build = {
    type = 'cmake';
    variables = {
        CMAKE_BUILD_TYPE="RelWithDebInfo";
        TARANTOOL_INSTALL_LIBDIR="$(LIBDIR)";
        TARANTOOL_INSTALL_LUADIR="$(LUADIR)";
    };
}
-- vim: syntax=lua ts=4 sts=4 sw=4 et
