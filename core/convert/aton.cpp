#include "core_config.h"
#include "conv.h"

#ifdef __cplusplus 
int aton(char *str, int *ret, char sep ){
#else
int aton(char *str, int *ret){
#endif
	unsigned char ch, radix, sgn=0;
	*ret = 0;
	radix= 0x0a;
	if ( ( ch=*str) == '-' ) {sgn=1; ++str;}   // ポインタを進める (旧 ++*str は入力文字を破壊するバグ)
	switch(str[1]){
	case 'x':
	case 'X':
		radix=0x10;
		str += 2;   // プレフィックス2文字 (例 "0x") をスキップ
		break;
	case 'b':
	case 'B':
		radix=0x02;
		str += 2;   // プレフィックス2文字 (例 "0x") をスキップ
		break;
	case 'd':
	case 'D':
		radix=0x0a;
		str += 2;   // プレフィックス2文字 (例 "0x") をスキップ
		break;
	case 'o':
	case 'O':
		radix=0x08;
		str += 2;   // プレフィックス2文字 (例 "0x") をスキップ
		break;
	default:
		break;
	}
#ifdef __cplusplus
	while ( (ch =*str++) && ch != sep ){
#else
	while ( (ch =*str++) ){
#endif
		ch -= 0x30;
		if ( ch >= 0x31 ) ch -= 0x20;
		if ( ch >= 0x11 ) ch -= 0x07;
		if ( ( ch <= 0x0f )) {
				*ret = *ret * radix + ch;
			}
		else return NUL_(ATON_INVALID_CH);
	}
	if (sgn) *ret= - *ret;
	return TRUE;
}

