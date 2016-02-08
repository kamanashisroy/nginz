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



int shotodol_scanner_next_token_delimitered_unused (aroop_txt_t* src, aroop_txt_t* next, aroop_txt_t* delim, aroop_txt_t* wordDivider) {
	unsigned int i = 0U;
	int token_start = 0;
	int trim_at = 0;
	int _tmp70_;
	int len = 0;
	aroop_wrong* _inner_error_ = NULL;
	int result = 0;
	i = (unsigned int) 0;
	token_start = -1;
	trim_at = -1;
	_tmp70_ = aroop_txt_length (src);
	len = _tmp70_;
	{
		aroop_bool _tmp71_ = false;
		i = (unsigned int) 0;
		_tmp71_ = true;
		while (true) {
			unsigned char _tmp72_;
			unsigned char x = '\0';
			aroop_bool _tmp73_ = false;
			aroop_bool _tmp74_ = false;
			if (!_tmp71_) {
				i++;
			}
			_tmp71_ = false;
			if (!(i < ((unsigned int) len))) {
				break;
			}
			_tmp72_ = aroop_txt_char_at (src, i);
			x = _tmp72_;
			if (wordDivider == NULL) {
				aroop_bool _tmp75_ = false;
				aroop_bool _tmp76_ = false;
				if (x == ((unsigned char) ' ')) {
					_tmp76_ = true;
				} else {
					_tmp76_ = x == ((unsigned char) '\r');
				}
				if (_tmp76_) {
					_tmp75_ = true;
				} else {
					_tmp75_ = x == ((unsigned char) '\n');
				}
				_tmp74_ = _tmp75_;
			} else {
				_tmp74_ = false;
			}
			if (_tmp74_) {
				_tmp73_ = true;
			} else {
				aroop_bool _tmp77_;
				_tmp77_ = aroop_txt_contains_char (wordDivider, x);
				_tmp73_ = _tmp77_;
			}
			if (_tmp73_) {
				if (token_start == (-1)) {
					continue;
				}
				trim_at = (int) i;
				break;
			} else {
				int _tmp78_ = 0;
				aroop_bool _tmp79_;
				if (token_start < 0) {
					_tmp78_ = (int) i;
				} else {
					_tmp78_ = token_start;
				}
				token_start = _tmp78_;
				_tmp79_ = aroop_txt_contains_char (delim, x);
				if (_tmp79_) {
					if (((unsigned int) token_start) == i) {
						i++;
					}
					trim_at = (int) i;
					break;
				}
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


int shotodol_scanner_next_token_delimitered (aroop_txt_t* src, aroop_txt_t* next, aroop_txt_t* delim) {
	unsigned int i = 0U;
	int token_start = 0;
	int trim_at = 0;
	int _tmp80_;
	int len = 0;
	aroop_wrong* _inner_error_ = NULL;
	int result = 0;
	i = (unsigned int) 0;
	token_start = -1;
	trim_at = -1;
	_tmp80_ = aroop_txt_length (src);
	len = _tmp80_;
	{
		aroop_bool _tmp81_ = false;
		i = (unsigned int) 0;
		_tmp81_ = true;
		while (true) {
			unsigned char _tmp82_;
			unsigned char x = '\0';
			int _tmp83_ = 0;
			aroop_bool _tmp84_;
			if (!_tmp81_) {
				i++;
			}
			_tmp81_ = false;
			if (!(i < ((unsigned int) len))) {
				break;
			}
			_tmp82_ = aroop_txt_char_at (src, i);
			x = _tmp82_;
			if (token_start < 0) {
				_tmp83_ = (int) i;
			} else {
				_tmp83_ = token_start;
			}
			token_start = _tmp83_;
			_tmp84_ = aroop_txt_contains_char (delim, x);
			if (_tmp84_) {
				if (((unsigned int) token_start) == i) {
					i++;
				}
				trim_at = (int) i;
				break;
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


int shotodol_scanner_next_token_delimitered_sliteral (aroop_txt_t* src, aroop_txt_t* next, aroop_txt_t* delim, aroop_txt_t* wordDivider) {
	unsigned int i = 0U;
	int token_start = 0;
	int trim_at = 0;
	int _tmp85_;
	int len = 0;
	aroop_bool stringLiteral = false;
	aroop_wrong* _inner_error_ = NULL;
	int result = 0;
	i = (unsigned int) 0;
	token_start = -1;
	trim_at = -1;
	_tmp85_ = aroop_txt_length (src);
	len = _tmp85_;
	stringLiteral = false;
	{
		aroop_bool _tmp86_ = false;
		i = (unsigned int) 0;
		_tmp86_ = true;
		while (true) {
			unsigned char _tmp87_;
			unsigned char x = '\0';
			aroop_bool isQuote = false;
			aroop_bool _tmp88_ = false;
			aroop_bool _tmp89_ = false;
			if (!_tmp86_) {
				i++;
			}
			_tmp86_ = false;
			if (!(i < ((unsigned int) len))) {
				break;
			}
			_tmp87_ = aroop_txt_char_at (src, i);
			x = _tmp87_;
			isQuote = x == ((unsigned char) '\"');
			if (stringLiteral) {
				_tmp89_ = isQuote;
			} else {
				_tmp89_ = false;
			}
			if (_tmp89_) {
				_tmp88_ = true;
			} else {
				aroop_bool _tmp90_ = false;
				if (!stringLiteral) {
					aroop_bool _tmp91_ = false;
					aroop_bool _tmp92_ = false;
					if (wordDivider == NULL) {
						aroop_bool _tmp93_ = false;
						aroop_bool _tmp94_ = false;
						aroop_bool _tmp95_ = false;
						if (x == ((unsigned char) ' ')) {
							_tmp95_ = true;
						} else {
							_tmp95_ = x == ((unsigned char) '\r');
						}
						if (_tmp95_) {
							_tmp94_ = true;
						} else {
							_tmp94_ = x == ((unsigned char) '\n');
						}
						if (_tmp94_) {
							_tmp93_ = true;
						} else {
							_tmp93_ = isQuote;
						}
						_tmp92_ = _tmp93_;
					} else {
						_tmp92_ = false;
					}
					if (_tmp92_) {
						_tmp91_ = true;
					} else {
						aroop_bool _tmp96_;
						_tmp96_ = aroop_txt_contains_char (wordDivider, x);
						_tmp91_ = _tmp96_;
					}
					_tmp90_ = _tmp91_;
				} else {
					_tmp90_ = false;
				}
				_tmp88_ = _tmp90_;
			}
			if (_tmp88_) {
				if (token_start == (-1)) {
					if (isQuote) {
						stringLiteral = true;
						token_start = ((int) i) + 1;
					}
					continue;
				} else {
					aroop_bool _tmp97_ = false;
					if (stringLiteral) {
						_tmp97_ = isQuote;
					} else {
						_tmp97_ = false;
					}
					if (_tmp97_) {
						trim_at = (int) i;
						i++;
						break;
					}
				}
				trim_at = (int) i;
				break;
			} else {
				int _tmp98_ = 0;
				aroop_bool _tmp99_ = false;
				if (token_start < 0) {
					_tmp98_ = (int) i;
				} else {
					_tmp98_ = token_start;
				}
				token_start = _tmp98_;
				if (!stringLiteral) {
					aroop_bool _tmp100_;
					_tmp100_ = aroop_txt_contains_char (delim, x);
					_tmp99_ = _tmp100_;
				} else {
					_tmp99_ = false;
				}
				if (_tmp99_) {
					if (((unsigned int) token_start) == i) {
						i++;
					}
					trim_at = (int) i;
					break;
				}
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


int shotodol_scanner_next_token (aroop_txt_t* src, aroop_txt_t* next) {
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


int shotodol_scanner_find_str (aroop_txt_t* src, aroop_txt_t* pattern) {
	int _tmp107_;
	int src_length = 0;
	int _tmp108_;
	int pattern_length = 0;
	aroop_wrong* _inner_error_ = NULL;
	int result = 0;
	_tmp107_ = aroop_txt_length (src);
	src_length = _tmp107_;
	_tmp108_ = aroop_txt_length (pattern);
	pattern_length = _tmp108_;
	{
		int i = 0;
		i = 0;
		{
			aroop_bool _tmp109_ = false;
			_tmp109_ = true;
			while (true) {
				int j = 0;
				if (!_tmp109_) {
					i++;
				}
				_tmp109_ = false;
				if (!(i <= (src_length - pattern_length))) {
					break;
				}
				j = 0;
				{
					aroop_bool _tmp110_ = false;
					j = 0;
					_tmp110_ = true;
					while (true) {
						aroop_bool _tmp111_ = false;
						if (!_tmp110_) {
							j++;
						}
						_tmp110_ = false;
						if (j < pattern_length) {
							unsigned char _tmp112_;
							unsigned char _tmp113_;
							_tmp112_ = aroop_txt_char_at (pattern, (unsigned int) j);
							_tmp113_ = aroop_txt_char_at (src, (unsigned int) (i + j));
							_tmp111_ = _tmp112_ == _tmp113_;
						} else {
							_tmp111_ = false;
						}
						if (!_tmp111_) {
							break;
						}
					}
				}
				if (j >= pattern_length) {
					result = i;
					return result;
				}
			}
		}
	}
	result = -1;
	return result;
}



