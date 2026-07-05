#include <stdarg.h>
#include "core_config.h"
#include "aout.h"
#include "conv.h"
/*
#ifdef __cplusplus
extern "C" {
#endif
*/
	int snprintf(char* str, unsigned int limit, const char* format, ...){
#include "printf_core.cpp"
		i=0;
		while (table[i] != 0 && (unsigned int)i < limit ) *str++=table[i++];
	if ( err > 0 ) return err;
	return TRUE;
}
/*
#ifdef __cplusplus
}
#endif
*/
