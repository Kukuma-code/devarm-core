#include <stdarg.h>
#include <stdint.h>   /* uintptr_t (printf_core の %p) */
#include "core_config.h"
#include "aout.h"
#include "conv.h"
/*
#ifdef __cplusplus
extern "C" {
#endif
*/
	int sprintf(char* str,const char* format, ...){
#include "printf_core.cpp"
		i=0;
		while (table[i] != 0 ) *str++=table[i++];
		*str=0;   /* 終端 NUL (C 標準。欠くと呼出側バッファが未終端のまま残る) */
	if ( err > 0 ) return (int)err;
	return (int)cnt;   /* 書いた文字数 (終端 NUL を除く。従来は TRUE 固定) */
}
/*
#ifdef __cplusplus
}
#endif
*/
