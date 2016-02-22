
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "log.h"
#include "net/streamio.h"
#include "net/http.h"
#include "net/http/http_plugin_manager.h"

C_CAPSULE_START

#define PAGE_CONTENT "<html>" \
"<head>" \
"<script type=\"text/javascript\">\n" \
"var chat = (function() {\n" \
	"return {\n" \
		"var xhr = new XMLHttpRequest();\n" \
		"xhr.open('POST', '/webchat', false);" \
		"xhr.setRequestHeader('Connection', 'Keep-Alive');" \
		"xhr.setRequestHeader('Keep-Alive', 'timeout=3600, max=3600');" \
		"xhr.onload = function (e) {\n" \
		  "if (xhr.readyState === 4) {\n" \
		    "if (xhr.status === 200) {\n" \
		      "console.log(xhr.responseText);\n" \
		      "document.getElementById(\"response_box\").value = xhr.responseText;\n" \
		    "} else {\n" \
		      "console.error(xhr.statusText);\n" \
		    "}\n" \
		  "}\n" \
		"};\n" \
		"xhr.onerror = function (e) {\n" \
		  "console.error(xhr.statusText);\n" \
		"};\n" \
		"submit : function() {\n" \
			"xhr.send(document.getElementById('chat_box').value);\n" \
			"return false;\n" \
		"}\n" \
	"};\n" \
"}());\n" \
"</script>\n" \
"</head>" \
"<body>" \
"<div>" \
"<textarea id=\"response_box\" rows=\"4\" cols=\"50\">" \
"Please write your name. The commands should be prefixed with '/' character. Enjoy." \
"</textarea>" \
"<form id=\"chat_form\" onsubmit=\"return chat.submit();\">" \
"<input type=\"text\" name=\"message\"/>" \
"<input id=\"chat_box\" type=\"submit\" name=\"send\"/>" \
"</form>" \
"</div>" \
"</body>" \
"</html>"

static int http_page_chat_plug(int signature, void*given) {
	aroop_assert(signature == HTTP_SIGNATURE);
	struct http_connection*http = (struct http_connection*)given;
	if(!IS_VALID_HTTP(http)) // sanity check
		return 0;
	aroop_txt_t page_content = {};
	aroop_txt_embeded_set_static_string(&page_content, PAGE_CONTENT);
	http->strm.send(&http->strm, &page_content, 0);
	return 0;
}

static int http_page_chat_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "pagechat", "http", plugin_space, __FILE__, "It is javascript based chat page.\n");
}

int page_chat_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "http/pagechat");
	composite_plug_bridge(http_plugin_manager_get(), &plugin_space, http_page_chat_plug, http_page_chat_desc);
	return 0;
}

int page_chat_module_deinit() {
	composite_unplug_bridge(http_plugin_manager_get(), 0, http_page_chat_plug);
	return 0;
}


C_CAPSULE_END
