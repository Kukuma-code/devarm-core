
#include "core_config.h"
#include "conv.h"
int ntoa(int val, char* result, int radix){
	int ret=1;
	switch (radix) {
	case 0x0a:
		ret=ntoad(val, result);
		break;
	case 0x10:
		ntoah(val, result);
		break;
	case 0x02:
		ntoab(val, result);
		break;
	default:
		return NUL_(NTOA_INVALID_RADIX);
		break;
	}
	return ret;
}
