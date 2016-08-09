#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memcached.h"
#include "mc_sasl.h"

/******************************************************************************/
/*                          wrappers on cyrus sasl lib                        */
/******************************************************************************/

#include <sasl/sasl.h>
#include <sasl/saslplug.h>

#include <tarantool/module.h>

const char mc_auth_ok_response[]   = "Authenticated";
size_t     mc_auth_ok_response_len = sizeof(mc_auth_ok_response) - 1;

char my_sasl_hostname[1025];

/******************************************************************************/
/*                   password database related functions                      */
/******************************************************************************/

#if 0
static const char *memcached_sasl_pwdb_c      = "MEMCACHED_SASL_PWDB";

#define MAX_ENTRY_LEN 256

static const char *memcached_sasl_pwdb;

static int sasl_check_token(char tkn) {
	return (tkn == ':' || tkn == '\n' || tkn == '\r' || tkn == '\0');
}

static int memcached_sasl_server_userdb_checkpass(
		sasl_conn_t *conn, void *context, const char *user,
		const char *pwd, unsigned pwd_len, struct propctx *propctx) {
	(void )conn;
	(void )context;
	(void )propctx;
	size_t user_len = strlen(user);
	if ((pwd_len + user_len) > (MAX_ENTRY_LEN - 4)) {
		say_error("<%s>: Too long request", __func__);
		return SASL_NOAUTHZ;
	}

	FILE *pwd_f = fopen(memcached_sasl_pwdb, "r");
	if (pwd_f == NULL) {
		say_error("<%s>: Failed to open database", __func__);
		return SASL_NOAUTHZ;
	}

	char buffer[MAX_ENTRY_LEN];
	bool ok = false;

	while (fgets(buffer, sizeof(buffer), pwd_f) != NULL) {
		if (memcmp(user, buffer, user_len) == 0 &&
		    buffer[user_len] == ':') {
			++user_len;
			if (memcmp(pwd, buffer + user_len, pwd_len) == 0 &&
			    sasl_check_token(buffer[user_len + pwd_len])) {
				ok = true;
			}
			break;
		}
	}
	if (ok == false) {
		say_error("<%s>: Failed to authenticate", __func__);
	}

	fclose(pwd_f);

	return (ok ? SASL_OK : SASL_NOAUTHZ);
}
#endif

#ifdef SASL_CB_GETCONF
static const char *memcached_sasl_conf_path_c = "SASL_CONF_PATH";

static const char *const memcached_sasl_getconf_locations[] = {
	"/etc/sasl/tarantool-memcached.conf",
	"/etc/sasl2/tarantool-memcached.conf",
	NULL
};

__attribute__((unused))
static int memcached_sasl_getconf(void *context, const char **path) {
	(void )context;
	*path = getenv(memcached_sasl_conf_path_c);

	if (*path == NULL) {
		for (int i = 0; memcached_sasl_getconf_locations[i] != NULL; ++i) {
			if (access(memcached_sasl_getconf_locations[i], F_OK) == 0) {
				*path = memcached_sasl_getconf_locations[i];
				break;
			}
		}
	}

	if (*path != NULL) {
		say_info("Found configuration file for SASL in '%s'", *path);
	} else {
		say_error("Can't find configuration file for SASL");
	}

	return (*path != NULL ? SASL_OK : SASL_FAIL);
}
#endif

static int memcached_sasl_log(void *context, int level, const char *message) {
	(void )context;
	int tnt_level = S_INFO;

	switch (level) {
	case SASL_LOG_PASS:
	case SASL_LOG_TRACE:
	case SASL_LOG_NOTE:
		tnt_level = S_INFO;
		break;
	case SASL_LOG_DEBUG:
	case SASL_LOG_NONE:
		tnt_level = S_DEBUG;
		break;
	case SASL_LOG_WARN:
		tnt_level = S_WARN;
		break;
	case SASL_LOG_FAIL:
		tnt_level = S_ERROR;
		break;
	default:
		/* unreacheable, becomes fatal */
		break;
	}

	say(tnt_level, NULL, "SASL <level %d> %s", level, message);

	return SASL_OK;
}

static sasl_callback_t sasl_callbacks[] = {
	{ SASL_CB_LOG, (sasl_callback_ft )&memcached_sasl_log, NULL},
#ifdef SASL_CB_GETCONF
	{ SASL_CB_GETCONF, (sasl_callback_ft )&memcached_sasl_getconf, NULL},
#endif
#if 0
	{
		SASL_CB_SERVER_USERDB_CHECKPASS,
		(sasl_callback_ft )&memcached_sasl_server_userdb_checkpass,
		NULL
	},
#endif
	{ SASL_CB_LIST_END, NULL, NULL},
};

/* PUBLIC API */

int memcached_sasl_init(void) {
	say_info("Initializing SASL: begin");

#if 0
	memcached_sasl_pwdb = getenv(memcached_sasl_pwdb_c);
	if (memcached_sasl_pwdb == NULL) {
		say_warn("PWDB isn't specified. skipping");
		sasl_callbacks[0].id   = SASL_CB_LIST_END;
		sasl_callbacks[0].proc = NULL;
	}
#endif

	memset(my_sasl_hostname, 0, sizeof(my_sasl_hostname));
	if (gethostname(my_sasl_hostname, sizeof(my_sasl_hostname) - 1) == -1) {
		say_syserror("Initializing SASL: Failed to discover hostname");
		my_sasl_hostname[0] = '\0';
	}

	if (sasl_server_init(sasl_callbacks, "tarantool-memcached") != SASL_OK){
		say_error("Initializing SASL: Failed");
		return -1;
	}

	say_info("Initializing SASL: done");
	return 0;
}

int memcached_sasl_connection_init(struct memcached_connection *con) {
	const char *hostname = NULL;
	if (my_sasl_hostname[0] != '\0') {
		hostname = (const char *)my_sasl_hostname;
	}
	struct sasl_ctx *ctx = con->sasl_ctx;
	int result = sasl_server_new("tarantool-memcached", NULL, hostname,
				     NULL, NULL, NULL, 0, &ctx->sasl_conn);
	if (result != SASL_OK) {
		say_error("Failed to initialize SASL");
		ctx->sasl_conn = NULL;
		return -1;
	}
	return 0;
}

int memcached_sasl_list_mechs(struct memcached_connection *con, const char **mechs,
			      size_t *mechs_len) {
	unsigned int mechs_len_ui = 0;
	struct sasl_ctx *ctx = con->sasl_ctx;
	int result = sasl_listmech(ctx->sasl_conn, NULL, "", " ", "", mechs,
				   &mechs_len_ui, NULL);
	if (result == SASL_OK) {
		*mechs_len = mechs_len_ui;
		return 0;
	}
	say_error("<%s>: Failed <sasl_listmech> with exit code %d", __func__,
		  result);
	*mechs = NULL;
	return -1;
}

/**
 * 0  means OK    - authentication done
 * 1  means STEP  - next step of authentication
 * -1 means ERROR
 */
int memcached_sasl_auth(struct memcached_connection *con, const char *mech,
			const char *challenge, size_t challenge_len,
			const char **out, size_t *out_len) {
	struct sasl_ctx *ctx = con->sasl_ctx;
	unsigned int challenge_len_ui = (unsigned int )challenge_len;
	unsigned int out_len_ui = 0;
	int result = sasl_server_start(ctx->sasl_conn, mech, challenge,
				       challenge_len_ui, out, &out_len_ui);
	*out_len = (size_t )out_len_ui;
	if (result == SASL_OK) {
		*out = mc_auth_ok_response;
		*out_len = mc_auth_ok_response_len;
		return 0;
	} else if (result == SASL_CONTINUE) {
		return 1;
	}
	say_error("<%s>: Failed <sasl_server_start> with exit code %d", __func__,
		  result);
	return -1;
}

/**
 * 0  means OK    - authentication done
 * 1  means STEP  - next step of authentication
 * -1 means ERROR
 */
int memcached_sasl_step(struct memcached_connection *con,
			const char *challenge, size_t challenge_len,
			const char **out, size_t *out_len) {
	struct sasl_ctx *ctx = con->sasl_ctx;
	unsigned int challenge_len_ui = (unsigned int )challenge_len;
	unsigned int out_len_ui = 0;
	int result = sasl_server_step(ctx->sasl_conn, challenge,
				       challenge_len_ui, out, &out_len_ui);
	*out_len = (size_t )out_len_ui;
	if (result == SASL_OK) {
		*out = mc_auth_ok_response;
		*out_len = mc_auth_ok_response_len;
		return 0;
	} else if (result == SASL_CONTINUE) {
		return 1;
	}
	say_error("<%s>: Failed <sasl_server_step> with exit code %d", __func__,
		  result);
	return -1;
}

