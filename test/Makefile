
FILES = $(wildcard *.txt)
WEBCHAT_SUFFIX=web_chat.txt
WEBCHAT_FILES+= greet_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= login_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= rooms_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= join_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= hi_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= profiler_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= version_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= leave_$(WEBCHAT_SUFFIX)
WEBCHAT_FILES+= quit_$(WEBCHAT_SUFFIX)
SED_CRLF=sed ':a;N;$$!ba;s/\n/\r\n/g'

all:

webchat:
	(for x in $(WEBCHAT_FILES); do \
		cat $$x | $(SED_CRLF); \
		sleep 1; \
	done) | nc localhost 80

websimple:
	cat $(WEBCHAT_SUFFIX) | $(SED_CRLF) | nc localhost 80

help:
	cat help.txt | nc localhost 80

