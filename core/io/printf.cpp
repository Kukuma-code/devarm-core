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
	//#ifndef ON_THE_HOST
//extern int putchar(int ch);
	//extern int puts(char *);
	//#endif
int printf(const char* format, ...){
#include "printf_core.cpp"
	puts(table);
	if ( err > 0 ) return (int)err;
	return (int)cnt;   /* 出力文字数 (C 標準。従来は TRUE 固定) */
}

/*
#ifdef __cplusplus
}
#endif
*/
