/*********************************************
 * platform/host/test: devarm core/mem/inspect + hal/mcheck の回帰ハーネス (host)
 * mcheck 継ぎ目 (hal/mcheck.h) 経由の checked_store/dump_word/dump_range をネイティブビルドし、
 *   - 継ぎ目が「可」を返すとき checked_store が書き込む
 *   - 継ぎ目を「不可」に差し替えると checked_store が書き込まない (ガードが効く)
 * を検証する。dump_word/dump_range は出力経路の疎通確認 (クラッシュしないこと)。
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert \
 *     -I core/mem -I . platform/host/test/mcheck_proof.cpp \
 *     core/convert/(all).cpp core/io/printf.cpp hal/io.cpp \
 *     platform/host/io_host.cpp platform/host/mcheck_host.cpp -o mcheck_proof
 * (printf は core/io の C++ linkage 版を使うため <cstdio> は include しない)
 *********************************************/
#include "inspect.h"

/* 継ぎ目を差し替えてガードを試すため、ポインタを直接触る */
extern "C" int (*mem_can_write)(const void*);
extern "C" int (*mem_can_read)(const void*);

static int deny(const void*){ return 0; }

static int failed = 0;
static void check(const char* name, bool ok){
	printf("[%s] %s\n", ok ? "OK" : "FAIL", name);
	if (!ok) ++failed;
}

int main(){
	// host 既定の継ぎ目は「可」
	check("host mcheck_write allows", mcheck_write((void*)&failed) != 0);
	check("host mcheck_read allows",  mcheck_read((void*)&failed) != 0);

	// 可のとき checked_store が書き込む (8/16/32bit)
	unsigned char  b8  = 0;
	unsigned short b16 = 0;
	unsigned int   b32 = 0;
	checked_store(0xEF,       &b8,                       1);
	checked_store(0xBEEF,     (unsigned char*)&b16,      2);
	checked_store(0xDEADBEEF, (unsigned char*)&b32,      4);
	check("checked_store writes 8bit",  b8  == 0xEF);
	check("checked_store writes 16bit", b16 == 0xBEEF);
	check("checked_store writes 32bit", b32 == 0xDEADBEEFu);

	// 継ぎ目を「不可」に差し替えると checked_store は書き込まない (ガード)
	mem_can_write = deny;
	unsigned int guarded = 0x11111111;
	checked_store(0x22222222, (unsigned char*)&guarded, 4);
	check("checked_store guard blocks write", guarded == 0x11111111u);
	// 以降 write は行わないため継ぎ目は deny のままでよい

	// 出力経路の疎通 (dump_word / dump_range) : クラッシュしないこと
	// mem_can_read は既定のまま (host = 可)
	unsigned int sample[4] = {0x01020304u, 0x05060708u, 0x090a0b0cu, 0x0d0e0f10u};
	dump_word(&sample[0], 4);
	dump_range(sample, sizeof(sample), 4, 0x10);
	check("dump_word/dump_range ran", true);

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
