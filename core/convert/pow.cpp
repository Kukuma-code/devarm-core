#include "math.h"
#ifndef TRUE
#define TRUE 1
#endif
// int pow(int base, int super){
// 	int i ,ret=0;
// 	if ( super==0 ) {
// 		ret=1;
// 		return TRUE;
// 	}
// 	ret=base;
// 	for ( i = 1; i < super ; i++){
// 		ret*=base;
// 	}
// 	return ret;
// }
// int pow(int base, int super, int *val){
// 	int i;
// 	if ( super==0 ) {
// 		*val=1;
// 		return TRUE;
// 	}
// 	*val=base;
// 	for ( i = 1; i < super ; i++){
// 		*val*=base;
// 	}
// 	return TRUE;
// }
double pow(double base, double super){
	double val;
	int sign=0, i;
	if ( super==0 ) {
		val=1;
		return val;
	}
	if ( super < 0 ) {
		super=-super; sign=1;
	}
	val=1;
	for ( i = 0; i < super ; i++){
		val*=base;
	}
	if ( sign ) { val= 1 / val ; }
	return (double)val;
}
