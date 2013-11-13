#ifndef EL__NETWORK_STATE_H
#define EL__NETWORK_STATE_H

struct terminal;

enum connection_priority {
	PRI_MAIN	= 0,
	PRI_DOWNLOAD	= 0,
	PRI_FRAME,
	PRI_CSS,
	PRI_NEED_IMG,
	PRI_IMG,
	PRI_PRELOAD,
	PRI_CANCEL,
	PRIORITIES,
};

/* Numbers < 0 and > -100000 are reserved for system errors reported via
 * errno/strerror(), see session.c and connection.c for further information. */
/* WARNING: an errno value <= -100000 may cause some bad things... */
/* NOTE: Winsock errors are in range 10000..11004. Hence our abs. values are
 * above this. */

#define is_system_error(state)		(S_OK < (state) && (state) < S_WAIT)
#define is_in_result_state(state)	((state) < 0)
#define is_in_progress_state(state)	((state) >= 0)
#define is_in_connecting_state(state)	(S_WAIT < (state) && (state) < S_TRANS)
#define is_in_transfering_state(state)	((state) >= S_TRANS)
#define is_in_queued_state(state)	(is_in_connecting_state(state) || (state) == S_WAIT)

/* FIXME: Namespace clash with Windows headers. */
#undef S_OK

enum connection_state {
	/* States >= 0 are used for connections still in progress. */
	S_WAIT			= 0,
	S_DNS,
	S_CONN,
	S_SSL_NEG,
	S_SENT,
	S_LOGIN,
	S_GETH,
	S_PROC,
	S_TRANS,
	S_QUESTIONS,
	S_CONN_PEERS,
	S_CONN_TRACKER,
	S_RESUME,

	/* State < 0 are used for the final result of a connection
	 * (it's finished already and it ended up like this) */
	S_OK			= -100000,
	S_INTERRUPTED		= -100001,
	S_EXCEPT		= -100002,
	S_INTERNAL		= -100003,
	S_OUT_OF_MEM		= -100004,
	S_NO_DNS		= -100005,
	S_CANT_WRITE		= -100006,
	S_CANT_READ		= -100007,
	S_MODIFIED		= -100008,
	S_BAD_URL		= -100009,
	S_TIMEOUT		= -100010,
	S_RESTART		= -100011,
	S_STATE			= -100012,
	S_WAIT_REDIR		= -100013,
	S_LOCAL_ONLY		= -100014,
	S_UNKNOWN_PROTOCOL	= -100015,
	S_EXTERNAL_PROTOCOL	= -100016,
	S_ENCODE_ERROR		= -100017,
	S_SSL_ERROR		= -100018,
	S_NO_FORCED_DNS		= -100019,

	S_HTTP_ERROR		= -100100,
	S_HTTP_204		= -100101,

	S_FILE_TYPE		= -100200,
	S_FILE_ERROR		= -100201,
	S_FILE_CGI_BAD_PATH	= -100202,
	S_FILE_ANONYMOUS	= -100203,

	S_FTP_ERROR		= -100300,
	S_FTP_UNAVAIL		= -100301,
	S_FTP_LOGIN		= -100302,
	S_FTP_PORT		= -100303,
	S_FTP_NO_FILE		= -100304,
	S_FTP_FILE_ERROR	= -100305,

	S_NNTP_ERROR		= -100400,
	S_NNTP_NEWS_SERVER	= -100401,
	S_NNTP_SERVER_HANG_UP	= -100402,
	S_NNTP_GROUP_UNKNOWN	= -100403,
	S_NNTP_ARTICLE_UNKNOWN	= -100404,
	S_NNTP_TRANSFER_ERROR	= -100405,
	S_NNTP_AUTH_REQUIRED	= -100406,
	S_NNTP_ACCESS_DENIED	= -100407,
	S_NNTP_SERVER_ERROR	= -100408,

	S_GOPHER_CSO_ERROR	= -100500,

	S_NO_JAVASCRIPT		= -100600,

	S_PROXY_ERROR		= -100700,

	S_BITTORRENT_ERROR	= -100800,
	S_BITTORRENT_METAINFO	= -100801,
	S_BITTORRENT_TRACKER	= -100802,
	S_BITTORRENT_BAD_URL	= -100803,
};

unsigned char *get_state_message(enum connection_state state, struct terminal *term);
void done_state_message(void);

#endif
