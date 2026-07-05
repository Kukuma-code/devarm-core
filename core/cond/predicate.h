#ifndef CORE_COND_PREDICATE_H
#define CORE_COND_PREDICATE_H
/*********************************************
 * core/cond: 文字/文字列の判定プリミティブ (移植可能・HW非依存)
 *
 * 旧 include/cond.h の純粋な判定ロジックを抽出し、密な 1 行記述を可読な
 * 形に整形したもの。ロジックは原典と同一 (下記の 2 点のバグ修正を除く)。
 *
 * 命名は現行の terse 名を維持 (rename は最終パスで一括実施):
 *   is<X>  … 単一文字の判定
 *   is<X>s … 文字列全体がすべて <X> かの判定
 *   is_num_str = is_digit_str (原典の別名)
 *
 * 原典からのバグ修正 (いずれもこの core 化で対処):
 *   1) `#ifdef _cplusplus` (誤) -> `#ifdef __cplusplus` (正)。
 *      原典はアンダースコアが 1 本足りず、int/char スカラ版
 *      (is_punct_str/is_arith_str/is_rel_str/is_logic_str/is_directive_str) が一度もコンパイル
 *      されないデッドコードだった。意図どおりオーバーロードを有効化する。
 *   2) 上記ブロック内の `NUK_(ISAUX)` (誤) -> `NUL_(ISAUX)` (正)。
 *      デッドコードだったため今まで顕在化していなかったタイポ。
 *
 * 注意 (libc 名衝突): is_digit / is_hex は標準 <ctype.h> と同名。
 *   このヘッダは <ctype.h>/<cctype> を include しないため単体では衝突しない
 *   が、両方を取り込む TU では衝突する。final rename パスで衝突しない名前
 *   へ寄せるまでは <ctype.h> と混在させないこと。
 *********************************************/
#include "core_config.h"   /* INLINE / TRUE / NUL_ */

/* --- 文字クラス判定マクロ (原典の CHECK_* をそのまま維持) --- */
#define CHECK_AMOUNT(str)   (('0'<=str && str<='9')||('A'<=str && str<='Z')||('a'<=str && str<='z')||('_'==str))
#define CHECK_DIGIT(var)    ('0'<=var && var<='9')
#define CHECK_DIGITS(var)   (('0'<=*var && *var<='9')||*var=='+'||*var=='-')
#define CHECK_XDIGIT(var)   (('0'<=var && var<='9')||('A'<=var && var<='F')||('a'<=var && var<='f'))
#define CHECK_AUX(str)      ((0x21<=str && str<=0x2f)||(0x3a<=str && str<=0x40)||(0x5b<=str && str<=0x60)||(0x7b<=str && str<=0x7e))
#define CHECK_OPERATOR(var) (0x25==var||0x2a==var||0x2b==var||0x2d==var||0x2f==var||0x3d==var)
#define CHECK_RELATIVE(var) (0x3c==var||0x3d==var||0x3e==var)
#define CHECK_LOGIC(var)    (0x26==var||0x5e==var||0x7c==var||0x7e==var)
#define CHECK_DIRECTOR(var) (0x23==var||0x24==var||0x40==var||0x5c==var)

/* --- 単一文字 / 文字列判定 (原典の active な定義) --- */

/* 英数字 + '_' (識別子構成文字) */
INLINE int is_ident_char(char str){
	if (!CHECK_AMOUNT(str)) return NUL_(ISAMOUNT);
	return TRUE;
}
INLINE int is_ident_str(char* str){
	while (*str){
		if (!CHECK_AMOUNT(*str)) return NUL_(ISAMOUNT);
		++str;
	}
	return TRUE;
}

/* 10 進数字 */
INLINE int is_digit(int ch){
	if (CHECK_DIGIT(ch)) return TRUE;
	return NUL_(ISDIGIT);
}
INLINE int is_digit_str(char* str){
	while (*str){
		if (!CHECK_DIGITS(str)) return NUL_(ISDIGITS);
		++str;
	}
	return TRUE;
}

/* 16 進数字 ("0x"/"0X" プレフィックスは is_hex_str でスキップ) */
INLINE int is_hex(int ch){
	if (!CHECK_XDIGIT(ch)) return NUL_(ISXDIGIT);
	return TRUE;
}
INLINE int is_hex_str(char* str){
	if (str[0]=='0' && (str[1]=='x' || str[1]=='X')) str += 2;
	while (*str){
		if (!CHECK_XDIGIT(*str)) return NUL_(ISXDIGITS);
		++str;
	}
	return TRUE;
}

/* 記号類 (aux) */
INLINE int is_punct_str(char* str){
	while (*str){
		if (!CHECK_AUX(*str)) return NUL_(ISAMOUNT);
		++str;
	}
	return TRUE;
}

/* 算術/関係/論理/ディレクティブ記号 (文字列版) */
INLINE int is_arith_str(char* str){
	while (*str){
		if (!CHECK_OPERATOR(*str)) return NUL_(ISARITH);
		++str;
	}
	return TRUE;
}
INLINE int is_rel_str(char* str){
	while (*str){
		if (!CHECK_RELATIVE(*str)) return NUL_(ISRELATIVE);
		++str;
	}
	return TRUE;
}
INLINE int is_logic_str(char* str){
	while (*str){
		if (!CHECK_LOGIC(*str)) return NUL_(ISLOGIC);
		++str;
	}
	return TRUE;
}
INLINE int is_directive_str(char* str){
	while (*str){
		if (!CHECK_DIRECTOR(*str)) return NUL_(ISDIRECTOR);
		++str;
	}
	return TRUE;
}

/* --- 単一文字スカラ版 (原典では _cplusplus タイポで無効化されていた) --- */
#ifdef __cplusplus
INLINE int is_punct_str(int ch){ if (!CHECK_AUX(ch)) return NUL_(ISAUX); return TRUE; }
INLINE int is_punct_str(char ch){ if (!CHECK_AUX(ch)) return NUL_(ISAUX); return TRUE; }
INLINE int is_arith_str(int ch){ if (!CHECK_OPERATOR(ch)) return NUL_(ISARITH); return TRUE; }
INLINE int is_arith_str(char ch){ if (!CHECK_OPERATOR(ch)) return NUL_(ISARITH); return TRUE; }
INLINE int is_rel_str(int ch){ if (!CHECK_RELATIVE(ch)) return NUL_(ISRELATIVE); return TRUE; }
INLINE int is_rel_str(char ch){ if (!CHECK_RELATIVE(ch)) return NUL_(ISRELATIVE); return TRUE; }
INLINE int is_logic_str(int ch){ if (!CHECK_LOGIC(ch)) return NUL_(ISLOGIC); return TRUE; }
INLINE int is_logic_str(char ch){ if (!CHECK_LOGIC(ch)) return NUL_(ISLOGIC); return TRUE; }
INLINE int is_directive_str(int ch){ if (!CHECK_DIRECTOR(ch)) return NUL_(ISDIRECTOR); return TRUE; }
INLINE int is_directive_str(char ch){ if (!CHECK_DIRECTOR(ch)) return NUL_(ISDIRECTOR); return TRUE; }
#endif /* __cplusplus */

/* 原典の別名: is_num_str = is_digit_str */
#define is_num_str(...) is_digit_str(__VA_ARGS__)

#endif /* CORE_COND_PREDICATE_H */
