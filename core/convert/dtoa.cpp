/*********************************************
 * dtoa: double -> ASCII 文字列変換 (固定小数点表記)
 *
 * 経緯:
 *   旧実装 (原典の移植) は 2 度修正した (ワードスワップ除去・null 終端)。
 *   Linux/x86-64 検証 (2026-10) で、なお次の誤りが残っていた:
 *     - 丸めが切り捨て桁へ加算しており、保持桁は繰り上がらない
 *       (%.1f 9.96 -> 9.9、%.2f 0.999 -> 0.99)。小数部から整数部への繰上げなし。
 *     - 2^-19 未満の値で隠れビット項 10^n/2^n が 64bit を桁あふれ (1e-10 が誤値)。
 *     - 2^52 以上で 64 以上のシフト量 (未定義動作)、2^52 付近で ':' を出力。
 *     - 0 / 非正規数 / inf / nan で pow(2,1023) の long long 変換 (未定義動作)。
 *   局所修正では塞がらないため、アルゴリズムを置き換えた:
 *
 * 方式 (正確変換):
 *   v = m * 2^e (m: 53bit 仮数) と分解する。整数部は m を e だけシフトした
 *   64bit 整数、小数部は分子 frac / 分母 2^k (k=-e, 最大 1074) を uint32 limb
 *   の多倍長で持ち、10 倍して k ビット目以上を取り出すことで 10 進桁を正確に
 *   得る。丸めは正確値に対する最近接偶数丸め (glibc の printf と同じ結果)。
 *   32bit ARM でも uint32 x 10 + 繰上げ (64bit 中間値) だけで動く。
 *
 * 仕様/制限:
 *   dtoa(ref, buf, _width): _width = 小数桁 (精度)。0 は既定 6 (従来互換)、負は絶対値。
 *   dtoa_prec(ref, buf, prec): prec=0 を許す (小数点なし, %.0f 用)。
 *   精度は DTOA_PREC_MAX (40) で頭打ち。出力最大長 = 符号 1 + 整数 20 + '.' + 40 + NUL
 *   = 63 バイト (printf の subtable 64 / putd の 0x44 に収まる)。
 *   |v| >= 2^64 は整数部が 64bit に収まらないため "ovf" (符号付き) を書き 0 を返す。
 *   inf / nan は "inf" / "nan" (負なら '-' 前置, glibc と同表記)。
 *   フィールド幅パディング (_pad) は未実装 (printf 側が幅を適用する)。
 *********************************************/
#include "core_config.h"
#include "conv.h"

#define DTOA_PREC_MAX 40
#define FRAC_LIMBS 36   /* 分母 2^1074 の分子 + 10 倍の桁上がり分 (uint32 x 36 = 1152bit) */

static void dtoa_put_u64(unsigned long long v, char* buf, long& j){
	char t[20]; int n=0;
	do { t[n++]=(char)('0' + v % 10); v /= 10; } while ( v != 0 );
	while ( n > 0 ) buf[j++]=t[--n];
}

static void dtoa_put_str(const char* s, char* buf, long& j){
	while ( *s ) buf[j++]=*s++;
}

int dtoa_prec(double ref, char* buf, int prec){
	unsigned long long bits;
	__builtin_memcpy(&bits, &ref, sizeof(bits));   /* IEEE754 binary64 を直接読む */
	int sign = (int)(bits >> 63);
	int ex   = (int)((bits >> 52) & 0x7ff);
	unsigned long long m = bits & ((1ull << 52) - 1);
	long j=0;
	if ( prec < 0 ) prec = -prec;
	if ( prec > DTOA_PREC_MAX ) prec = DTOA_PREC_MAX;
	if ( sign ) buf[j++]='-';
	if ( ex == 0x7ff ) {
		dtoa_put_str(m ? "nan" : "inf", buf, j); buf[j]=0;
		return TRUE;
	}
	int e;
	if ( ex == 0 ) e = -1074;                     /* 0 / 非正規数 */
	else { m |= 1ull << 52; e = ex - 1075; }      /* 正規数: 隠れビットを付与 */

	unsigned long long ip;                        /* 整数部 */
	unsigned int frac[FRAC_LIMBS];                /* 小数部の分子 (分母 2^k) */
	for ( int i=0; i<FRAC_LIMBS; ++i ) frac[i]=0;
	int k=0;
	if ( e >= 0 ) {
		if ( e > 11 ) {                           /* m < 2^53 なので e<=11 で < 2^64 */
			dtoa_put_str("ovf", buf, j); buf[j]=0;
			return NUL_(DTOA_RANGE_OVER);
		}
		ip = m << e;
	} else {
		k = -e;
		ip = ( k < 64 ) ? ( m >> k ) : 0;
		unsigned long long f = ( k < 64 ) ? ( m & ((1ull << k) - 1) ) : m;
		frac[0]=(unsigned int)f; frac[1]=(unsigned int)(f >> 32);
	}

	/* 小数 prec 桁 + 丸め判定用の次の 1 桁を生成。rest は以降に非零が残るか。 */
	char dg[DTOA_PREC_MAX];
	int nxt=0, rest=0;
	if ( k ) {
		int w = k / 32, b = k % 32;               /* k ビット目 = limb w の bit b */
		for ( int d=0; d<=prec; ++d ) {
			unsigned long long c=0;
			for ( int i=0; i<=w+1; ++i ) {
				c += (unsigned long long)frac[i] * 10u;
				frac[i]=(unsigned int)c; c >>= 32;
			}
			unsigned int digit;
			if ( b == 0 ) { digit = frac[w]; frac[w]=0; }
			else {
				digit = (frac[w] >> b) | (frac[w+1] << (32 - b));
				frac[w] &= (1u << b) - 1u; frac[w+1]=0;
			}
			if ( d < prec ) dg[d]=(char)digit; else nxt=(int)digit;
		}
		for ( int i=0; i<FRAC_LIMBS; ++i ) if ( frac[i] ) { rest=1; break; }
	} else {
		for ( int d=0; d<prec; ++d ) dg[d]=0;
	}

	/* 最近接偶数丸め */
	int last = prec ? dg[prec-1] : (int)(ip & 1);
	if ( nxt > 5 || ( nxt == 5 && ( rest || (last & 1) ) ) ) {
		int i=prec-1;
		for ( ; i>=0; --i ) { if ( ++dg[i] < 10 ) break; dg[i]=0; }
		if ( i < 0 ) ++ip;                        /* 小数部から整数部へ繰上げ (e<0 なので ip < 2^53) */
	}

	dtoa_put_u64(ip, buf, j);
	if ( prec ) {
		buf[j++]='.';
		for ( int d=0; d<prec; ++d ) buf[j++]=(char)('0' + dg[d]);
	}
	buf[j]=0;
	return TRUE;
}

int dtoa(double ref, char* buf, int _width, int _pad){
	(void)_pad;   /* フィールド幅パディングは未実装 (ファイル冒頭参照) */
	if ( _width < 0 ) _width = -_width;
	if ( _width == 0 ) _width = 6;
	return dtoa_prec(ref, buf, _width);
}
#undef DTOA_PREC_MAX
#undef FRAC_LIMBS
