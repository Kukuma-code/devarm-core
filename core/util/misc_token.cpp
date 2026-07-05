/*********************************************
 * core/util: is_token_name の実体 (宣言・マクロは misc_token.h)
 * {value,name} 表を線形走査し value 一致 (または番兵 -1) で name を返す。
 * アルゴリズムは原典と同一。
 *********************************************/
#include "misc_token.h"

const char* is_token_name(long _value, token_* _ref){
	long idx = 0;
	while (1){
		if ( _ref[idx].value == _value ) break;
		if ( _ref[idx].value == -1 ) break;
		++idx;
	}
	return _ref[idx].name;
}
