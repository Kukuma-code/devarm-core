/*********************************************
 * platform/host/test: devarm core/block の回帰ハーネス (host)
 * core/block/byte.h をネイティブビルドし、byte_copy/byte_fill が標準
 * memcpy/memset と一致することを確認する。
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/block \
 *     platform/host/test/block_proof.cpp -o block_proof
 *********************************************/
#include "byte.h"
#include <cstring>
#include <cstdio>

static int failed = 0;

static void check(const char* name, bool ok){
	printf("%-16s %s\n", name, ok ? "OK" : "FAIL");
	if (!ok) ++failed;
}

int main(){
	const char* msg = "sample bytes\0with embedded NUL";
	const unsigned int n = 30;   // 埋め込み NUL を跨いでコピーする

	// byte_copy vs memcpy
	{
		unsigned char a[64], b[64];
		memset(a, 0xAA, sizeof(a)); memset(b, 0xAA, sizeof(b));
		byte_copy(msg, a, n);
		memcpy(b, msg, n);
		check("byte_copy==memcpy", memcmp(a, b, sizeof(a)) == 0);
	}

	// byte_fill vs memset
	{
		unsigned char a[64], b[64];
		byte_fill(0x5C, a, 40);
		memset(b, 0x5C, 40);
		// 埋めた範囲外は未初期化なので比較は 40 バイトのみ
		check("byte_fill==memset", memcmp(a, b, 40) == 0);
	}

	// cnt==0 は no-op (境界)
	{
		unsigned char a[8]; memset(a, 0x11, sizeof(a));
		byte_copy(msg, a, 0); byte_fill(0x22, a, 0);
		bool untouched = true;
		for (unsigned int i = 0; i < sizeof(a); ++i) if (a[i] != 0x11) { untouched = false; break; }
		check("cnt==0 noop", untouched);
	}

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
