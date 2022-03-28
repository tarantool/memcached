# Hacking

Here we collect arcane knowledge, which may be useful for developers of the
module.

## memcached_ibuf

`third_party/memcached_ibuf.[ch]` is the copy of ibuf from the small library
with renaming of the functions. The motivation is to avoid possible name clash
between tarantool's symbols and the module's symbols. See [1] and [2] for
details.

We can remove it, when we'll dedice to drop support of tarantool versions
affected by [1] or will find another way to overcome the name clash (see [3]).

[1]: https://github.com/tarantool/tarantool/issues/6873
[2]: https://github.com/tarantool/memcached/issues/59#issuecomment-1081106140
[3]: https://github.com/tarantool/memcached/issues/92#issuecomment-1081128938
