
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"

C_CAPSULE_START



int default_streamio_send(struct streamio*strm, aroop_txt_t*content, int flag) {
	if(strm->next)
		return strm->next->send(strm->next, content, flag);
	aroop_assert(strm->fd != INVALID_FD);
	return send(strm->fd, aroop_txt_to_string(content), aroop_txt_length(content), flag);
}

int default_streamio_close(struct streamio*strm) {
	if(strm->next) {
		return strm->next->close(strm->next);
	}
	if(strm->fd != INVALID_FD) {
		event_loop_unregister_fd(strm->fd);
		close(strm->fd);
		strm->fd = -1;
	}
	return 0;
}


C_CAPSULE_END

