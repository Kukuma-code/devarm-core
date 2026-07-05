#include "core_config.h"
#include "format.h"
#define BUF_SZ 0x24
#ifdef __cplusplus
extern "C" {
#endif
#define ISSIGN 0x1
#define ISRIGHT 0x2
int ntoad(int _val, char* _result, int _width, int _pad){
	int tmp, status=0, i;
	char buf[BUF_SZ];
	if ( _width < 0 ) {status|=ISRIGHT; _width=-_width;}
	for ( i=0; i<BUF_SZ; i++) buf[i]=0;
	i=0;
	if (_val == 0 ){*_result ='0'; return TRUE;}
	if (_val==(int)0x80000000) { // lowest value can't reverse normally.
		status|=ISSIGN; _val+=1; _val=-_val; tmp=_val%10; tmp+=0x31;
		buf[i++]=(char)tmp; _val=_val/10;}
	if ( _val < 0 ) { _val =-_val;	status=1;	}
	do { tmp= _val % 10; tmp += 0x30; buf[i++]=(char)tmp; _val = _val / 10;
	} while ( _val != 0 );
	return putn_formats(_result,buf,i,status,_width,_pad);
}
// #define ret _val
// 	ret=i;
// 	if ( _width == 0 ) {
// 		if (status & ISSIGN){buf[i]='-';++ret;} else --i;
// 		while ( i >= 0 ) { *_result++=buf[i--]; }
// 	}
// 	else {
// 		if ( _width < i ) {ret=i;	if ( status & ISSIGN ) ++ret;
// 		} else ret=_width;
// 		if ( _pad == '0' ) { if (status & ISSIGN) {*_result++='-';--i;} else --i;}
// 		else {if (status & ISSIGN) {buf[i]='-';++_width;} else --i;}
// #define pad_width tmp
// 		if (status & ISSIGN) pad_width=_width-i-2; else pad_width=_width-i-1;
// 		DBGD(_width);DA();DBGD(i);DN();
// 		if ( status & ISSIGN && ( _width == i+1) ) ++ret;
// 		if ( !(status & ISRIGHT) ) for ( ; pad_width>0; --pad_width ) *_result++=_pad;
// 		for ( ; buf[i] != 0 ; --i ) *_result++=buf[i];
// 		if ( (status & ISRIGHT) ) for ( ; pad_width>0; --pad_width) *_result++=_pad;
// 	}
// 	*_result=0;
// #undef pad_width
// 	return ret;
// }
// #undef ret
#undef BUF_SZ
#undef ISSIGN
#undef ISRIGHT
#ifdef __cplusplus
}
#endif
