/*********************************************
 * platform/host/test: devarm core/cond の回帰ハーネス (host)
 * core/cond/predicate.h をネイティブビルドし、判定結果が期待値と一致する
 * ことを確認する。is_digit/is_hex は libc <ctype.h> と同名のため、この
 * ハーネスは <cctype> を include せず自前の参照式で検証する。
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/cond \
 *     platform/host/test/cond_proof.cpp -o cond_proof
 *********************************************/
#include "predicate.h"
#include <cstdio>

static int failed = 0;

static void check(const char* name, bool ok){
	printf("%-24s %s\n", name, ok ? "OK" : "FAIL");
	if (!ok) ++failed;
}

/* 参照式 (libc に依存しない) */
static bool ref_digit(int c){ return c>='0' && c<='9'; }
static bool ref_xdigit(int c){ return (c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f'); }

int main(){
	// is_digit(int) : 全 256 コードポイントで参照式と一致
	{
		bool ok = true;
		for (int c = 0; c < 256; ++c)
			if ((is_digit(c) != 0) != ref_digit(c)) { ok = false; break; }
		check("is_digit==ref", ok);
	}

	// is_hex(int) : 全 256 コードポイントで参照式と一致
	{
		bool ok = true;
		for (int c = 0; c < 256; ++c)
			if ((is_hex(c) != 0) != ref_xdigit(c)) { ok = false; break; }
		check("is_hex==ref", ok);
	}

	// is_digit_str (旧テスト isdigit_test.c の入力: +/- を許容する点に注意)
	check("is_digit_str(\"15\")",   is_digit_str((char*)"15"));
	check("is_digit_str(\"-1\")",   is_digit_str((char*)"-1"));    // 符号のみでも各文字が +/-/digit なら真
	check("is_digit_str(\"a1\")==0", !is_digit_str((char*)"a1"));
	check("is_digit_str(\"@f\")==0", !is_digit_str((char*)"@f"));

	// is_num_str = is_digit_str (別名)
	check("is_num_str(\"0123456789\")", is_num_str((char*)"0123456789"));
	check("is_num_str(\"01_23.0~\")==0", !is_num_str((char*)"01_23.0~"));

	// is_hex_str ("0x" プレフィックスをスキップ)
	check("is_hex_str(\"0x1a\")", is_hex_str((char*)"0x1a"));
	check("is_hex_str(\"15\")",   is_hex_str((char*)"15"));
	check("is_hex_str(\"@f\")==0", !is_hex_str((char*)"@f"));
	check("is_hex_str(\"1x00\")==0", !is_hex_str((char*)"1x00"));  // 先頭 "0x" でないため x が不正

	// is_ident_char / is_ident_str (英数字 + '_')
	check("is_ident_char('A')",  is_ident_char('A'));
	check("is_ident_char('_')",  is_ident_char('_'));
	check("is_ident_char('#')==0", !is_ident_char('#'));
	check("is_ident_str(\"sample_testTEST\")", is_ident_str((char*)"sample_testTEST"));
	check("is_ident_str(\"a b\")==0", !is_ident_str((char*)"a b"));

	// is_arith_str (算術記号) : スカラ版は原典タイポで無効だったが有効化済み
	check("is_arith_str('+')",   is_arith_str('+'));
	check("is_arith_str('a')==0", !is_arith_str('a'));
	check("is_arith_str(\"+-/*\")", is_arith_str((char*)"+-/*"));

	// is_punct_str / is_rel_str / is_logic_str / is_directive_str (有効化したスカラ版)
	check("is_punct_str('#')",       is_punct_str('#'));
	check("is_rel_str('<')",  is_rel_str('<'));
	check("is_logic_str('&')",     is_logic_str('&'));
	check("is_directive_str('@')",  is_directive_str('@'));

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
