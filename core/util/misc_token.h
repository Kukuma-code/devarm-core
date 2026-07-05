#ifndef CORE_UTIL_MISC_TOKEN_H
#define CORE_UTIL_MISC_TOKEN_H
/*********************************************
 * core/util: token 名解決ヘルパ (移植可能・外部依存なし)
 *
 * 旧 devel lib/util/misc_token.c + include/misc_token.h を host 移植したもの。
 * {value, name} の配列を線形に引き、一致した value の name を返すだけの小物。
 * TK(x) = {x, #x} でシンボル名を文字列化してテーブルを作り、TK_END() が番兵 (-1)。
 * elf クラスタ (ELF ヘッダの token ダンプ) の前提依存。単体で自己完結。
 *
 * 原典からの変更 (C++17 対応):
 *   name / 戻り値を char* -> const char* に修正。TK(x) の #x は文字列リテラル
 *   (const char[]) で、char* への変換は C++11 以降不可 (-Wall -Wextra でエラー/警告)。
 *********************************************/
struct token_ {
	long value;
	const char* name;
};

const char* is_token_name(long _value, token_* _ref);

#define TK(x) {x,#x}
#define TK_END() {-1,"UNKNOWN"}

#endif /* CORE_UTIL_MISC_TOKEN_H */
