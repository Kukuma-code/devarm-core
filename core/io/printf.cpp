#include <stdarg.h>
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
	if ( err > 0 ) return err;
	return TRUE;
}

/*
#ifdef __cplusplus
}
#endif
*/
