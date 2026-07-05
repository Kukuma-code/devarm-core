/*********************************************
 * dtoa: double -> ASCII 文字列変換
 *
 * FIXED (著者 TODO の「dtoa に bug」を解消。原因は2つ):
 *   1. IEEE754 ビット取得時の 32bit ワードスワップ。X86->ARM 移植時に旧 ARM
 *      (FPA/OABI) のワード順対策として混入したが、標準 IEEE (ホスト/EABI
 *      little-endian) では不要で、符号・指数の抽出を破壊していた。
 *      -> 標準レイアウトを memcpy で直接読むよう修正。
 *   2. 出力 buf の null 終端漏れ。printf 経由 (ゼロ初期化バッファ) では露見
 *      しないが putd の未初期化バッファ渡しで末尾ゴミが出ていた。
 *      -> 末尾に buf[j]=0 を追加。
 *   検証: putd / printf("%f") ともシステム printf と一致
 *         (platform/host/test/io_proof.cpp)。
 *
 * 仕様/制限:
 *   _width = 小数点以下の桁数(精度, 既定 6)。フィールド幅パディングと右詰め
 *   (_pad) は未実装。旧版の 2 引数 dtoa (raw_dtoa.c) は未使用のため core から除外。
 *********************************************/
// #undef DEBUG
// #ifdef DBG_FPRINTD
// #define DEBUG
// #endif

#ifdef DBG_DTOA_ENABLE
#define DBG_PTAB() print_array(table, TABLE_DIGIT)
#define DBG_PTAB_SUB() print_array(subtable, REAL_DIGIT)
#define DBG_BITD(x) print_bitd(x)
#else
#define DBG_PTAB()
#define DBG_PTAB_SUB()
#define DBG_BITD(x)
#endif
#include "core_config.h"
#include "math.h"
#include "conv.h"
#undef TABLE_DIGIT
#define TABLE_DIGIT 0x50
#define REAL_DIGIT 0x34
#define SUPER_DIGIT 0x0b
#define SIGN_DIGIT 0x01
#define ISSIGN 0x01
#define ISRIGHT 0x02
#define ISESIGN 0x04
// |S| -- SUPER_DIGIT -- | -- REAL_DIGIT -- |
int dtoa(double ref, char* buf, int _width, int _pad){
	FUNC_IN();
	long i, j;
	unsigned long long tmpbit;
	unsigned long long b=0;
	unsigned long long c, bias=0;
	//	unsigned long long b;
	unsigned long long radix=10;
	char table[TABLE_DIGIT];
	char subtable[TABLE_DIGIT];
	int sign, e_sign=0;
	long long super;
	long long pow_sp, pow_rl;
	unsigned long long int_p=0 , small;
	// _width は小数桁(精度)。フィールド幅パディング(_pad/右詰め)は未実装のため
	// _pad は現状未使用。負の _width は精度の絶対値として扱う。
	(void)_pad;
	if ( _width < 0 ) _width = -_width;
	for ( i=0; i<TABLE_DIGIT; i++) {
		table[i]=0;	
		subtable[i]=0;
	}
	// IEEE754 double のビットパターンを取得する。
	// 旧コードは上下 32bit ワードを入れ替えていた (X86->ARM 移植時に、旧 ARM の
	// FPA/OABI では double のワード順が逆になる対策として混入)。しかし標準 IEEE754
	// (ホスト、および ARM EABI little-endian) ではスワップ不要で、これが putd 破損の
	// 原因だった。ここでは標準レイアウトを直接読む (strict-aliasing 安全に memcpy)。
	__builtin_memcpy(&tmpbit, &ref, sizeof(tmpbit));
	DBG_DTOA("value : ");DBG_BITD(tmpbit);
	sign=(int) ( tmpbit >> ( DOUBLE_DIGIT - 1 ) ) ;
	DBG_DTOA_D(DBGDN(sign));
	SLIGHTIN();
	small = tmpbit << 1;
	//	DBG("bit << 1 : ");DBG_BITD(small);
	super = ( small >> ( REAL_DIGIT + 1 ) ) ;
	DBG("super bit : ");DBG_BITD(super);
	DBGLD(super);DA();DBGLD((long long)pow(2,SUPER_DIGIT-1)-1);DN();
	super = super - ( (long long)pow(2, SUPER_DIGIT - 1 )  - 1)  ;
	DBG("[Ajust] super = super - ( pow(2,0xb-1) -1)\n");DBGLD(super);DA();DBG_BITD(super);
	if ( super < 0 ) { e_sign=1; super=-super; }
	pow_sp=(long long)pow(2,super);
	DBGD(e_sign) ;DN();
	DBG("[Identify Small part] : "); 
	if ( e_sign ) {
		DBG("***** Small part only. *****");DN();
		small= tmpbit << ( DOUBLE_DIGIT - REAL_DIGIT );
		small= small >> ( DOUBLE_DIGIT - REAL_DIGIT ) ;
	}
	else {
		DBG("***** Integer part has. *****");DN();
		small= tmpbit << ( ( DOUBLE_DIGIT - REAL_DIGIT + super ) );
		small= small >> ( DOUBLE_DIGIT - REAL_DIGIT + super );
		if ( super ){
			int_p = tmpbit << ( DOUBLE_DIGIT - REAL_DIGIT );
			int_p = int_p >> ( ( DOUBLE_DIGIT - REAL_DIGIT ) + ( REAL_DIGIT - super ));
		}
		DBG_DTOA("int part   :");DBG_BITD(int_p);
	}
	DBG_DTOA("small part :");DBG_BITD(small);
	DBG("[Preparation]<END>"); DN();
	SLIGHTIN();
	DBG("[Small part process 1]<IN>");DBGN();
	if ( small != 0 ){
		DBG_DTOA("[Small part has]<IN>\n");
		if ( !e_sign ) b=small << super;
		else { b=small ;}
		i=0;c=1;
		pow_rl=(long long)pow(2,REAL_DIGIT);
		while ( c != 0 ){
			b= b*radix;
			c= b % pow_rl;
			b= b / pow_rl;
			table[i]=b;
			b=c ;
			i++;
		}
		DBG_DTOA("          : 0.");DBG_PTAB();
		DBG("[Small part process 1]<OUT>");DBGN();
		if ( e_sign ){
#define mod bias
		DBG("[Small part process 2]<IN>");DBGN();
		DBG("small part divided by pow_sp=(long long)pow(2,super)::");DBGLD(pow_sp);DBGN();
		i=0;
		while ( table[i] == 0 ) i++;
		//		printf("i : %d\n", i);
		while ( i < TABLE_DIGIT ){	c= (mod*radix) + table[i];
			b = c / pow_sp; mod = c % pow_sp;	table[i]=b;
			//		printf("%d : %d , next mod : %d\n", i, table[i], mod);
			i++; }
		//		putchar('\n');
		DBG_DTOA("small part table: 0.");DBG_PTAB();
		DBG("[Small part process 2]<OUT>");DBGN();
		}
	}
	if ( e_sign ){
		SLIGHTIN();
		DBG("[Small part process3]<IN>");DBGN();
		DBG("1 divided by 2 at 'super number' times.: %d\n", super);
		c=super;b=1;
		while ( c != 0 ){	b= b*radix;	b= b / 2;	c--; }
		DBG("trans value : %ld , ",b);
		c= pow_sp; bias=-1;
		while ( c != 0 ){	c=c/radix; ++bias; }
		DBG("small digit bias : %ld \n", bias);
		c=b; super=0;
		while ( c != 0 ){ subtable[super]=c%radix; c=c/radix;	++super; }
		DBG("pow_sp : %d , bias : %d\n", pow_sp, bias);
		DBG("b : %d , ref_digit : %d\n", b, super);
		
		DBG_PTAB_SUB();
		DBG("[Small part process]<OUT>");DBGN();
		DBG("[Sum process2 and process3]<IN>");DN();
		b=0;
		for ( j=super-1, i=0 ; i<super ; i++, j--){
			////		DBG("i :%d , table : %d, sub : %d\n", i, table[bias+j], subtable[i]);
			table[bias+j]+=b+subtable[i];
			if ( table[bias+j] >= 10 ) {
				table[bias+j]-=10; b=1; }
			else b=0;
		}
		if ( b ) table[bias+j]+=b;
		DBG_PTAB();
		DBG("[Sum process2 and process3]<OUT>");DN();
	} //e_sign

	SLIGHTIN();
	DBG_PTAB();
	DBG("[Last process]");DBGN();
	j=0;
	//	//	printf("%f\n", ref);
	if ( sign ) buf[j++]='-';
	if ( !e_sign ) {
		DBG_DTOA("[Int part calc]<IN>");DN();
		DBGLD(int_p);DA();DBGLD(pow_sp);DN();
#ifdef __cplusplus
		if ( !lntoa(int_p+pow_sp, &buf[j])) return NUL_(PRINTD_LNTOA);
#else
		if ( !lntoa(int_p+pow_sp, &buf[j], 0x0a )) return NUL_(PRINTD_LNTOA);
#endif
		while ( buf[j] != 0 ) j++;
		buf[j++]='.';
		DBGS(buf);DBGN();
		DBG_DTOA("[Int part calc]<OUT>");DN();
	} else { buf[j++]='0'; buf[j++]='.'; }
	//	simple round.

	if ( _width == 0 ) _width=6;
	if ( table[_width]+5 > 10 ){
		b=1;
		for ( i=_width; i>=0; i--){
			table[i]+=b;
			if ( table[i] >= 10 ) {table[i]=0; b=1;}
			else break;
		}
	}
	for ( i=0; i<_width; i++ ) buf[j++]=table[i]+0x30;
	buf[j]=0;   // null 終端 (未初期化バッファ渡しでの末尾ゴミ出力を防ぐ)
// 	if ( table[6]+5 > 10 ){
// 		b=1;
// 		for ( i=6; i>=0; i--){
// 			table[i]+=b;
// 			if ( table[i] >= 10 ) {table[i]=0; b=1;}
// 			else break;
// 		}
// 	}
// 	for ( i=0; i<6; i++ ) buf[j++]=table[i]+0x30;

	//	printf("%s\n", buf );DBGN();
	/*
	for ( i=0 ; i<7 ; i++) printf("%d", table[i]);
	*/
	FUNC_OUT();
	return TRUE;
}
#undef TABLE_DIGIT
#undef REAL_DIGIT
#undef SUPER_DIGIT
#undef SIGN_DIGIT
