#ifndef CONV_H
#define CONV_H

#include "core_config.h"
#include "format.h"
#ifdef __cplusplus
extern int printf(const char* format,...);
extern "C"{
#endif
#ifdef __cplusplus
INLINE void ntoah(int val, char* result,int _width=8){if ( _width == 0 || (_width=_width*4) > LONG_DIGIT) _width=LONG_DIGIT ;for (int j=_width-4; j >=0 ; j=j-4){
	_width = ( val >> j) & 0x0f;	if ( _width >= 0x0a ) _width += 0x27;	_width += 0x30;*result++=_width;}	*result=0;}
INLINE void untoah(unsigned int val, char* result,int _width=8){if ( _width == 0 || (_width=_width*4) > LONG_DIGIT) _width=LONG_DIGIT ;for (int j=_width-4; j >=0 ; j=j-4){
	_width = ( val >> j) & 0x0f;	if ( _width >= 0x0a ) _width += 0x27;	_width += 0x30;*result++=_width;}	*result=0;}
INLINE void ntoab(int val, char* result,int _width=0x20){if ( _width == 0 || _width > LONG_DIGIT) _width=LONG_DIGIT ;for(int j=_width-1; j>=0 ; j--){
	_width = ( val >> j ) & 0x01;_width += 0x30;*result++=_width;} *result=0;}
INLINE void untoab(unsigned int val, char* result,int _width=0x20){if ( _width == 0 || _width > LONG_DIGIT) _width=LONG_DIGIT ;for(int j=_width-1; j>=0 ; j--){
	_width = ( val >> j ) & 0x01;_width += 0x30;*result++=_width;} *result=0;}
nINLINE	int ntoad(int val, char* result, int _width=0, int _pad=0x20);
nINLINE	int untoad(unsigned int val, char* result, int _width=0, int _pad=0x20);
nINLINE	int ntoadv(int val, char* result, int _width=0, int _pad=0x20);
nINLINE	int ntoa(int val, char* result, int radix=0x0a);
INLINE void lntoah(long long val, char* result,int _width=0x10){if (_width == 0 || (_width*=4) > DOUBLE_DIGIT) _width=DOUBLE_DIGIT ;for (int j=_width-4; j >=0 ; j=j-4){_width = ( val >> j) & 0x0f;if (_width >= 0x0a) _width += 0x27;_width += 0x30;*result++=_width;} *result=0;}
INLINE void ulntoah(unsigned long long val, char* result,int _width=0x10){if (_width == 0 || (_width*=4) > DOUBLE_DIGIT) _width=DOUBLE_DIGIT ;for (int j=_width-4; j >=0 ; j=j-4){_width = ( val >> j) & 0x0f;if (_width >= 0x0a) _width += 0x27;_width += 0x30;*result++=_width;} *result=0;}
INLINE void lntoab(long long val, char* result,int _width=0x40){if (_width == 0 || _width > DOUBLE_DIGIT) _width=DOUBLE_DIGIT ;for(int j=_width-1; j>=0 ; j--){_width = ( val >> j ) & 0x01;_width += 0x30;*result++=_width;} *result=0;}
INLINE void ulntoab(unsigned long long val, char* result,int _width=0x40){if (_width == 0 || _width > DOUBLE_DIGIT) _width=DOUBLE_DIGIT ;for(int j=_width-1; j>=0 ; j--){_width = ( val >> j ) & 0x01;_width += 0x30;*result++=_width;} *result=0;}
nINLINE	int lntoad(long long val, char* result, int _width=0, int _pad=0x20);
nINLINE	int ulntoad(unsigned long long val, char* result, int _width=0, int _pad=0x20);
nINLINE	int lntoadv(long long val, char* result, int _width=0, int _pad=0x20);
	//	int lntoadf(long long val, char* result, int _width=0, int _pad=0x20);
nINLINE	int lntoa(long long val, char* result, int radix=0x0a);
nINLINE	int dtoa(double ref, char* buf,int _width=6, int _pad=0x20);
nINLINE	int dtoa_prec(double ref, char* buf, int prec);   /* prec 0 可 (小数点なし) */
// 	int dtoa(double ref, char* buf,int _width=6, int _small_width=3, int _pad=0x20);
nINLINE	int aton(char *str, int *ret, char sep= 0x20);
#else
nINLINE	int ntoa(int val, char* result, int radix);
	//	int lntoa(int val, char* result, int radix);
nINLINE	int lntoa(long long val, char* result, int radix);
nINLINE	int dtoa(double ref, char* buf);
nINLINE	int dtoa_prec(double ref, char* buf, int prec);
nINLINE	int aton(char *str, int *ret);
#endif
#ifdef __cplusplus
}
#endif
#endif
