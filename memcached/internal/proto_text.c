const mc_process_func_t memcached_txt_handler[] = {
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_SET,     0x00 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_ADD,     0x01 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_REPLACE, 0x02 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_APPEND,  0x03 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_PREPEND, 0x04 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_CAS,     0x05 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_GET,     0x06 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_GETS,    0x07 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_DELETE,  0x08 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_INCR,    0x09 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_DECR,    0x0a */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_FLUSH,   0x0b */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_STATS,   0x0c */
	eemcached_process_unsupported, /* MEMCACHED_TXT_CMD_VERSION, 0x0d */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_QUIT,    0x0e */
};


int
memcached_text_write(struct memcached_connection *con, uint16_t err,
		     uint64_t cas, uint8_t ext_len, uint16_t key_len,
		     uint32_t val_len, const char *ext,
		     const char *key, const char *val)
{
	return 0;
}


/**
 * Check that we need transaction for our operation.
 */
static inline int
memcached_text_ntxn(struct memcached_connection *con)
{
	uint8_t cmd = con->hdr->cmd;
	if ((cmd >= MEMCACHED_TXT_CMD_SET &&
	     cmd <= MEMCACHED_TXT_CMD_CAS) ||
	    (cmd >= MEMCACHED_TXT_CMD_DELETE &&
	     cmd <= MEMCACHED_TXT_CMD_DECR))
		return 1;
	return 0;
};

int
memcached_binary_process(struct memcached_connection *con) {
	int rv = 0;
	/* Process message */
	con->noreply = false;
	if (memcached_text_ntxn(con)) {
		box_txn_begin();
	}
	if (con->hdr->cmd < MEMCACHED_BIN_CMD_MAX) {
		rv = memcached_txt_handler[con->->cmd](con);
		if (box_txn()) box_txn_commit();
	} else {
		rv = memcached_process_unknown(con);
	}
	return rv;
}
