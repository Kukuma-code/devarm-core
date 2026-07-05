#include <stdarg.h>
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
	if ( err > 0 ) return err;
	return TRUE;
}
/*
#ifdef __cplusplus
}
#endif
*/
