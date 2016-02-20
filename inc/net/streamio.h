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
	/**
	 * The fd of the stream. If there is no fd then it is set to -1/INVALID_FD.
	 */
	int fd;
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
	 * It is the next aggregated stream. In one way cases, it is garbage collected. It needs not be garbage collected in two way situation.
	 */
	struct streamio*next;
};

int default_streamio_send(struct streamio*strm, aroop_txt_t*content, int flag);
int default_streamio_close(struct streamio*strm);

#define IS_VALID_STREAM(x) ((x)->fd != INVALID_FD || (x)->next != NULL)

C_CAPSULE_END

#endif // NGINZ_STREAM_IO_H
