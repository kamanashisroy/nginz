#ifndef NGINZ_STREAM_IO_H
#define NGINZ_STREAM_IO_H

C_CAPSULE_START

enum {
	INVALID_FD = -1,
};

/**
 * This is streaming/pipeline contruct. It allows the piping for filtering, tunneling or other purposes.
 * It is possible to implement proxy, delegator, bridge pattern using this contruct. 
 */

struct streamio {
	struct opp_object_ext _ext; // so that it is searchable
	/**
	 * The fd of the stream. If there is no fd then it is set to -1/INVALID_FD.
	 */
	int fd;
	/**
	 * On data receive ..
	 */
	int (*on_recv)(struct streamio*strm, aroop_txt_t*content);
	/**
	 * It sends/writes data to stream.
	 * @param flags It is generally the flags available for send application namely, MSG_MORE, MSG_DONTWAIT ..
	 */
	int (*send)(struct streamio*strm, aroop_txt_t*content, int flags);
	/**
	 * It closes the stream and removes the fd(if available) from the event loop. It disconnects itself(possibly destroying) from the next stream.
	 */
	int (*close)(struct streamio*strm);
	/**
	 * It is the next aggregated stream. In one way cases, it is garbage collected. It needs not be garbage collected in two way situation(avoid circular reference). It facilitates chain-of-responsibility pattern.
	 */
	struct streamio*bubble_up;
	struct streamio*bubble_down;
	/**
	 * It transfers a file descriptor to another processor
	 */
	int (*transfer_parallel)(struct streamio*strm, int destpid, int proto_port, aroop_txt_t*cmd);
	/**
	 * nonblocking send buffer
	 */
	aroop_txt_t send_buffer;
	int error; // last error
};

int default_streamio_send(struct streamio*strm, aroop_txt_t*content, int flag);
int default_streamio_send_nonblock(struct streamio*strm, aroop_txt_t*content, int flag);
int default_streamio_close(struct streamio*strm);
int default_transfer_parallel(struct streamio*strm, int destpid, int proto_port, aroop_txt_t*cmd);
int streamio_initialize(struct streamio*strm);
int streamio_chain(struct streamio*up, struct streamio*down);
int streamio_unchain(struct streamio*up, struct streamio*down);
int streamio_finalize(struct streamio*strm);

#define IS_VALID_STREAM(x) ((x)->fd != INVALID_FD || (x)->bubble_up != NULL)

C_CAPSULE_END

#endif // NGINZ_STREAM_IO_H
