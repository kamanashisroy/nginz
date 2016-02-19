#ifndef NGINZ_WEB_CHAT_H
#define NGINZ_WEB_CHAT_H

C_CAPSULE_START

/**
 * `web_chat_connection` overrides `http_connection`.
 * Again it faciliatates proxy for `chat_connection`. 
 */
struct web_chat_connection {
	struct http_connection http;
	struct chat_connection*chat;
}

C_CAPSULE_END

#endif // NGINZ_WEB_CHAT_H
