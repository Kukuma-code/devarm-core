/*********************************************
 * platform/host/test: devarm core/mem の回帰ハーネス (host)
 * core/mem/raw.h をネイティブビルドし、mem_fill/mem_compare が標準 memset/memcmp と、
 * store8/store16/store32 が型付きストアとして一致することを確認する。
 * (mem_fill/mem_compare は libc と別名のため <cstring> 混在でも衝突しない)
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/mem \
 *     platform/host/test/mem_proof.cpp -o mem_proof
 *********************************************/
#include "raw.h"
#include <cstring>
#include <cstdio>

static int failed = 0;

static void check(const char* name, bool ok){
	printf("%-20s %s\n", name, ok ? "OK" : "FAIL");
	if (!ok) ++failed;
}

/* mem_compare の符号だけを標準 memcmp と突き合わせる (差分の大きさは実装依存) */
static int sgn(int x){ return (x > 0) - (x < 0); }

int main(){
	// mem_fill vs memset
	{
		unsigned char a[64], b[64];
		memset(a, 0xAA, sizeof(a)); memset(b, 0xAA, sizeof(b));
		mem_fill('A', a, 40);
		memset(b, 'A', 40);
		check("mem_fill==memset", memcmp(a, b, sizeof(a)) == 0);
	}

	// mem_compare vs memcmp : 一致・前後差・複数長で符号一致
	{
		const char* s1 = "test1";
		const char* s2 = "test2";
		bool ok = true;
		for (unsigned int n = 1; n <= 5; ++n){
			int ret; mem_compare(s1, s2, n, ret);
			if (sgn(ret) != sgn(memcmp(s1, s2, n))) { ok = false; break; }
		}
		check("mem_compare sign==memcmp", ok);

		int r; mem_compare("abc", "abc", 3, r);
		check("mem_compare equal==0", r == 0);
		mem_compare("abd", "abc", 3, r);
		check("mem_compare greater>0", r > 0);
		mem_compare("abc", "abd", 3, r);
		check("mem_compare less<0", r < 0);
	}

	// store8/store16/store32 : 型付きストア
	{
		unsigned char  b8;  store8(0xEF, &b8);
		unsigned short b16; store16(0xBEEF, &b16);
		unsigned int   b32; store32(0xDEADBEEFu, &b32);
		check("store8", b8 == 0xEF);
		check("store16", b16 == 0xBEEF);
		check("store32", b32 == 0xDEADBEEFu);
	}

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
