#ifndef CORE_CONVERT_FORMAT_H
#define CORE_CONVERT_FORMAT_H
/*********************************************
 * core/convert: 数値整形ヘルパ putn_formats (純粋・HW非依存)
 *
 * 符号/幅/パディングを適用してバッファに書き出す。ntoad などの 10 進変換が
 * 使う。旧 include/a.h にあったが、conv 側の依存であるため convert へ移動した。
 *********************************************/
#include "core_config.h"

#define ISSIGN  0x1
#define ISRIGHT 0x2

INLINE unsigned int putn_formats(char* buf, char* sub, int _cnt, int _status=0, int _width=0, int _pad=0x20){
	int ret=_cnt, pad_width;
	if ( _width == 0 ) {
		if (_status & ISSIGN){sub[_cnt]='-';++ret;} else --_cnt;
		while (_cnt >= 0) { *buf++=sub[_cnt--];}}
	else {
		if (_width < _cnt) {ret=_cnt;	if (_status & ISSIGN) ++ret;} else ret=_width;
		if (_pad == '0') { if (_status & ISSIGN) {*buf++='-';--_cnt;} else --_cnt;}
		else {if (_status & ISSIGN) {sub[_cnt]='-';++_width;} else --_cnt;}
		if (_status & ISSIGN) pad_width=_width-_cnt-2; else pad_width=_width-_cnt-1;
		if (_status & ISSIGN && (_width == _cnt+1)) ++ret;
		if ( !(_status & ISRIGHT) ) for ( ; pad_width>0; --pad_width) *buf++=_pad;
		for ( ; sub[_cnt] != 0 ; --_cnt) *buf++=sub[_cnt];
		if ( (_status & ISRIGHT) ) for ( ; pad_width>0; --pad_width) *buf++=_pad;
	}
	*buf=0;	return ret;}

#undef ISSIGN
#undef ISRIGHT
#endif /* CORE_CONVERT_FORMAT_H */
