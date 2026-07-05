
#include "core_config.h"
#include "conv.h"
int lntoa(long long val, char* result, int radix){
	int ret=1;
	switch (radix) {
	case 0x0a:
		ret=lntoad(val,result);
		break;
	case 0x10:
		lntoah(val, result);
		break;
	case 0x02:
		lntoab(val, result);
		break;
	default:
		return NUL_(NTOA_INVALID_RADIX);
		break;
	}
	return ret;
}
