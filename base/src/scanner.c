// NOTE this is generated code from aroop compiler, please do not edit this.

#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "scanner.h"
#include <ctype.h>

int scanner_trim (aroop_txt_t* src, aroop_txt_t* trimmed) {
	aroop_txt_embeded_txt_copy_shallow(trimmed, src);
	int len = aroop_txt_length(trimmed);
	while(len--) {
		char ch = aroop_txt_char_at(trimmed, len);
		if(!isspace(ch)) {
			aroop_txt_set_length(trimmed, (len+1));
			return (len+1);
		}
	}
	aroop_txt_set_length(trimmed, 0);
	return 0;
}

int scanner_next_token (aroop_txt_t* src, aroop_txt_t* next) {
	unsigned int i = 0U;
	int token_start = 0;
	int trim_at = 0;
	int _tmp101_;
	int len = 0;
	aroop_wrong* _inner_error_ = NULL;
	int result = 0;
	i = (unsigned int) 0;
	token_start = -1;
	trim_at = -1;
	_tmp101_ = aroop_txt_length (src);
	len = _tmp101_;
	{
		aroop_bool _tmp102_ = false;
		i = (unsigned int) 0;
		_tmp102_ = true;
		while (true) {
			unsigned char _tmp103_;
			unsigned char x = '\0';
			aroop_bool _tmp104_ = false;
			aroop_bool _tmp105_ = false;
			if (!_tmp102_) {
				i++;
			}
			_tmp102_ = false;
			if (!(i < ((unsigned int) len))) {
				break;
			}
			_tmp103_ = aroop_txt_char_at (src, i);
			x = _tmp103_;
			if (x == ((unsigned char) ' ')) {
				_tmp105_ = true;
			} else {
				_tmp105_ = x == ((unsigned char) '\r');
			}
			if (_tmp105_) {
				_tmp104_ = true;
			} else {
				_tmp104_ = x == ((unsigned char) '\n');
			}
			if (_tmp104_) {
				if (token_start == (-1)) {
					continue;
				}
				trim_at = (int) i;
				break;
			} else {
				int _tmp106_ = 0;
				if (token_start < 0) {
					_tmp106_ = (int) i;
				} else {
					_tmp106_ = token_start;
				}
				token_start = _tmp106_;
			}
		}
	}
	if (token_start >= 0) {
		aroop_txt_embeded_rebuild_copy_shallow (next, src);
		if (trim_at >= 0) {
			aroop_txt_truncate (next, (unsigned int) trim_at);
		}
		aroop_txt_shift (next, token_start);
	} else {
		aroop_txt_truncate (next, (unsigned int) 0);
	}
	aroop_txt_shift (src, (int) i);
	result = 0;
	return result;
}



