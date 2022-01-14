-- memcached.lua

local ffi    = require('ffi')
local socket = require('socket')
local errno  = require('errno')
local uri    = require('uri')
local log    = require('log')

local fmt = string.format

local internal_so_path = package.search('memcached.internal')
assert(internal_so_path, "Failed to find memcached/internal.so library")
local memcached_internal = ffi.load(internal_so_path)

ffi.cdef[[
struct memcached_stat {
    /* connection informations */
    unsigned int  curr_items;
    unsigned int  total_items;
    unsigned int  curr_conns;
    unsigned int  total_conns;
    uint64_t      bytes_read;
    uint64_t      bytes_written;
    /* get statistics */
    uint64_t      cmd_get;
    uint64_t      get_hits;
    uint64_t      get_misses;
    /* delete stats */
    uint64_t      cmd_delete;
    uint64_t      delete_hits;
    uint64_t      delete_misses;
    /* set statistics */
    uint64_t      cmd_set;
    uint64_t      cas_hits;
    uint64_t      cas_badval;
    uint64_t      cas_misses;
    /* incr/decr stats */
    uint64_t      cmd_incr;
    uint64_t      incr_hits;
    uint64_t      incr_misses;
    uint64_t      cmd_decr;
    uint64_t      decr_hits;
    uint64_t      decr_misses;
    /* touch/flush stats */
    uint64_t      cmd_touch;
    uint64_t      touch_hits;
    uint64_t      touch_misses;
    uint64_t      cmd_flush;
    /* expiration stats */
    uint64_t      evictions;
    uint64_t      reclaimed;
    /* authentication stats */
    uint64_t      auth_cmds;
    uint64_t      auth_errors;
};

void
memcached_set_opt (struct memcached_service *srv, int opt, ...);

enum memcached_proto_type {
    MEMCACHED_PROTO_NEGOTIATION = 0x00,
    MEMCACHED_PROTO_BINARY      = 0x01,
    MEMCACHED_PROTO_TEXT        = 0x02,
    MEMCACHED_PROTO_MAX
};

enum memcached_options {
    MEMCACHED_OPT_READAHEAD      = 0x00,
    MEMCACHED_OPT_EXPIRE_ENABLED = 0x01,
    MEMCACHED_OPT_EXPIRE_COUNT   = 0x02,
    MEMCACHED_OPT_EXPIRE_TIME    = 0x03,
    MEMCACHED_OPT_FLUSH_ENABLED  = 0x04,
    MEMCACHED_OPT_VERBOSITY      = 0x05,
    MEMCACHED_OPT_PROTOCOL       = 0x06,
    MEMCACHED_OPT_SASL           = 0x07,
    MEMCACHED_OPT_MAX
};

struct memcached_stat *
memcached_get_stat (struct memcached_service *);

struct memcached_service *
memcached_create(const char *, uint32_t);

int    memcached_start     (struct memcached_service *);
void   memcached_stop      (struct memcached_service *);
void   memcached_free      (struct memcached_service *);
void   memcached_handler   (struct memcached_service *p, int fd);
int    memcached_setsockopt(int fd, const char *family, const char *type);
]]

local function startswith(str, start)
       return string.sub(str, 1, string.len(start)) == start
end

local typetable = {
    name = {
        'string',
        function () return 'memcached' end,
        function (_) return true end,
        [[Name of memcached instance]]
    },
    uri = {
        'string',
        function() return '0.0.0.0:11211' end,
        function(x) local a = uri.parse(x); return (a and a['service']) end,
        [[The read/write data URI]]
    },
    readahead = {
        'number',
        function() return box.cfg.readahead end,
        function(x) return x > 0 and x < math.pow(2, 10) end,
        [[size of readahead buffer]]
    },
    expire_enabled = {
        'boolean',
        function() return true end,
        function(_) return true end,
        [[configure availability of expiration daemon]]
    },
    expire_items_per_iter = {
        'number',
        function() return 200 end,
        function(x) return x > 0 end,
        [[configure scan count for expiration]]
    },
    expire_full_scan_time = {
        'number',
        function() return 3600 end,
        function(x) return x > 0 end,
        [[time required for full index scan (in seconds)]]
    },
    verbosity = {
        'number',
        function() return 0 end,
        function(x) return x > 0 and x < 4 end,
        [[verbosity of memcached logging]]
    },
    if_not_exists = {
        'boolean',
        function() return false end,
        function(x) return x end,
        [[do not throw error if exists]]
    },
    space_name = {
        'string',
        function() return nil end,
        function(x) return x end,
        [[custom name of space to use/create]]
    },
    protocol = {
        'string',
        function() return 'negotiation' end,
        function(x)
            return x == 'negot' or
                   x == 'negotiation' or
                   x == 'bin' or
                   x == 'binary' or
                   x == 'ascii' or
                   x == 'text'
        end,
        [[protocol type ('negotiation'/'binary'/'ascii')]]
    },
    sasl = {
        'boolean',
        function() return false end,
        function(x) return x end,
        [[enable SASL authorization]]
    },
    storage = {
        'string',
        function() return 'memory' end,
        function(x)
            return x == 'mem' or
                   x == 'memory' or
                   x == 'disk'
        end,
        [[storage type ('memory' for RAM, 'disk' for HDD/SSD)]]
    }
--    flush_enabled = {
--        'boolean',
--        function() return true end,
--        [[flush command availability]]
--    },
}

local err_enomem          = "Can't allocate memory for %s"
local err_bad_args        = "Arguments must be in dictionary"
local err_bad_box         = "Please call box.cfg{} first"
local err_no_such_option  = "No such option '%s'"
local err_bad_option_type = "Bad '%s' option type, expected '%s', got '%s'"
local err_bad_value       = "Bad value for argument '%s'"
local err_bad_instance    = "Instance with name '%s' is already created"
local err_is_stopped      = "Memcached instance '%s' is already stopped"
local err_is_started      = "Memcached instance '%s' is already started"

local function config_check(cfg)
    for k, v in pairs(cfg) do
        if typetable[k] == nil then
            return false, fmt(err_no_such_option, k)
        end
        if type(v) ~= typetable[k][1] then
            return false, fmt(err_bad_option_type, k, typetable[k][1], type(v))
        end
        if not typetable[k][3](v) then
            return false, fmt(err_bad_value, k)
        end
    end
    return true
end

local function config_initial(cfg)
    local newcfg = {}
    for k, v in pairs(typetable) do newcfg[k] = v[2]() end
    for k, v in pairs(cfg) do newcfg[k] = v end
    local stat, err = config_check(cfg)
    if stat then return newcfg end
    box.error{ reason = err }
end

-- TODO: This function is unused, but I see a good idea behind it: show
-- available options and its default values. Perhaps someday we will use it.
-- luacheck: no unused
local function config_help()
    for k, v in pairs(typetable) do
        log.info('%s: %s', k, v[4])
        log.info("%s type is '%s'",    string.rep(' ', #k + 1), v[1])
        log.info("%s default is '%s'", string.rep(' ', #k + 1), tostring(v[2]()))
    end
end
local memcached_services = {}
local RUNNING = 'r'
local STOPPED = 's'
local ERRORED = 'e'

local stat_table = {
    'total_items', 'curr_items',
    'curr_conns', 'total_conns',
    'bytes_read', 'bytes_written',
    'cmd_get', 'get_hits', 'get_misses',
    'cmd_delete', 'delete_hits', 'delete_misses',
    'cmd_set', 'cas_hits', 'cas_badval', 'cas_misses',
    'cmd_incr', 'incr_hits', 'incr_misses',
    'cmd_decr', 'decr_hits', 'decr_misses',
    'cmd_touch', 'touch_hits', 'touch_misses',
    'cmd_flush',
    'evictions', 'reclaimed',
    'auth_cmds', 'auth_errors'
}

local C = ffi.C

local conf_table = {
    readahead             = C.MEMCACHED_OPT_READAHEAD,
    expire_enabled        = C.MEMCACHED_OPT_EXPIRE_ENABLED,
    expire_items_per_iter = C.MEMCACHED_OPT_EXPIRE_COUNT,
    expire_full_scan_time = C.MEMCACHED_OPT_EXPIRE_TIME,
    verbosity             = C.MEMCACHED_OPT_VERBOSITY,
    protocol              = C.MEMCACHED_OPT_PROTOCOL,
    sasl                  = C.MEMCACHED_OPT_SASL
}

local memcached_methods = {
    cfg = function (self, opts)
        if type(opts) ~= 'table' then
            error(err_bad_args)
        end
        local stat, err = config_check(opts or {})
        if stat == false then
            error(err)
        end
        for k, v in pairs(opts) do
            if conf_table[k] ~= nil then
                memcached_internal.memcached_set_opt(self.service,
                                                     conf_table[k], v)
                self.opts[k] = v
            end
        end
        return self
    end,
    start = function (self)
        local function memcached_handler(socket, addr)
            log.debug('client %s:%s connected', addr.host, addr.port)
            memcached_internal.memcached_handler(self.service, socket:fd())
        end
        jit.off(memcached_handler)

        if self.status == RUNNING then
            error(fmt(err_is_started, self.name))
        end
        memcached_internal.memcached_start(self.service)
        local parsed = uri.parse(self.uri)
        self.listener = socket.tcp_server(parsed.host, parsed.service, {
            handler = memcached_handler
        })
        local lname = self.listener:name()
        if self.listener == nil then
            self.status = ERRORED
            error(fmt('can\'t bind (%d) %s', errno(), errno.strerror()))
        end
        if (memcached_internal.memcached_setsockopt(self.listener:fd(),
                                   lname.family,
                                   lname.type) == -1) then
            self.status = ERRORED
            error(fmt('can\'t set options (%d) %s', errno(), errno.strerror()))
        end
        self.status = RUNNING
        return self
    end,
    stop = function (self)
        if self.status == STOPPED then
            error(fmt(err_is_stopped, self.name))
        end
        if (self.listener ~= nil) then
            self.listener:close()
        end
        memcached_internal.memcached_stop(self.service)
        self.status = STOPPED
        return self
    end,
    info = function (self)
        local stats = memcached_internal.memcached_get_stat(self.service)
        local retval = {}
        for _, v in pairs(stat_table) do
            retval[v] = stats[0][v]
        end
        return retval
    end,
    grant = function (self, username)
        box.schema.user.grant(username, 'read,write', 'space', self.space_name)
        return self
    end
}

local function memcached_init(name, uri, opts)
    opts = opts or {}
    if memcached_services[name] ~= nil then
        if not opts.if_not_exists then
            error(fmt(err_bad_instance, name))
        end
        return memcached_services[name]
    end
    if type(box.cfg) == 'function' then
        error(fmt(err_bad_box, name))
    end
    local conf = config_initial(opts)
    local instance = {}
    instance.opts = conf
    instance.name = name
    instance.uri  = uri
    instance.space_name = opts.space_name or '__mc_' .. instance.name
    if box.space[instance.space_name] == nil then
        local storage = startswith(conf.storage, 'mem') and 'memtx' or 'vinyl'
        local index   = startswith(conf.storage, 'mem') and 'hash' or nil
        instance.space = box.schema.create_space(instance.space_name, {
            engine = storage,
            format = {
                { name = 'key',      type = 'str' },
                { name = 'expire',   type = 'num' },
                { name = 'creation', type = 'num' },
                { name = 'value',    type = 'str' },
                { name = 'cas',      type = 'num' },
                { name = 'flags',    type = 'num' },
            }
        })
        instance.space:create_index('primary', {
            parts = {1, 'str'},
            type = index
        })
    else
        instance.space = box.space[instance.space_name]
    end
    local service = memcached_internal.memcached_create(instance.name,
                                                        instance.space.id)
    if service == nil then
        error(fmt(err_enomem, "memcached service"))
    end
    instance.service = ffi.gc(service, memcached_internal.memcached_free)
    memcached_services[instance.name] = setmetatable(instance, {
        __index = memcached_methods
    })
    return instance:cfg(opts):start()
end

local function memcached_get(name)
    return memcached_services[name]
end

return {
    create = memcached_init,
    get    = memcached_get,
    server = setmetatable({}, {
        __index = memcached_services
    })
}
