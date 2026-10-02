/*********************************************
 * platform/host/test: devarm core 出力パスの回帰ハーネス (host)
 * conv + io + hal + host backend をネイティブビルドして動作確認する。
 * ビルド例:
 *   clang++ -std=c++17 -I core/include -I core/convert -I core/io -I . \
 *     platform/host/test/io_proof.cpp core/convert/(all .cpp) \
 *     core/io/printf.cpp core/io/sprintf.cpp core/io/snprintf.cpp \
 *     hal/io.cpp platform/host/io_host.cpp -o io_proof
 * (aton.cpp は入力系で未整形のため本ハーネスの対象外)
 *********************************************/
#include "aout.h"

/* 64bit (LP64) 回帰: 期待文字列と一致しなければ [FAIL] を出す (runner が検出) */
static bool same(const char* a, const char* b){
	while ( *a && *a == *b ) { ++a; ++b; }
	return *a == *b;
}
static void expect(const char* name, const char* got, const char* want){
	printf("[%s] %s: %s\n", same(got, want) ? "OK" : "FAIL", name, got);
}
/* スタックに非ゼロの上位 32bit を残し、可変長引数の読み幅違いを顕在化させる */
__attribute__((noinline)) static void dirty_stack(){
	volatile unsigned long long junk[64];
	for (int i = 0; i < 64; ++i) junk[i] = 0xdeadbeef00000000ull;
	(void)junk;
}

int main(){
	// 整数 / 16進 / 2進 / long long
	putn(-42);   putchar('\n');   // -42
	puth(255,2); putchar('\n');   // ff
	putb(5,8);   putchar('\n');   // 00000101
	putln(-1LL); putchar('\n');   // -1

	// printf / sprintf
	printf("d=%d x=%x s=%s c=%c u=%u b=%b\n", 42, 255, "hi", 'Z', 7u, 5);
	char sb[64]; sprintf(sb, "sp[%d,%x]", 7, 0xab); puts(sb); putchar('\n');

	// double (dtoa) : システム printf と一致するはず
	putd(3.14159, 6); putchar('\n');           // 3.141590
	putd(-0.25, 6);   putchar('\n');           // -0.250000
	printf("f=%f g=%f h=%f\n", 3.14159, -2.5, 100.125);

	// aton (ascii -> number, 入力系): 符号・基数プレフィックス
	{ char b[]="-0x1F"; int r=0; aton(b,&r); printf("aton(-0x1F)=%d\n", r); }  // -31
	{ char b[]="0b101"; int r=0; aton(b,&r); printf("aton(0b101)=%d\n", r); }  // 5

	// LP64: %u は unsigned int で読む (6 個目以降はスタック渡し)
	{ char b[64]; dirty_stack();
	  sprintf(b, "%u %u %u %u %u %u %u %u %u", 1u,2u,3u,4u,5u,6u,7u,8u,9u);
	  expect("%u x9 (stack args)", b, "1 2 3 4 5 6 7 8 9"); }
	// sprintf / snprintf の終端 NUL (snprintf は limit に NUL を含む)
	{ char b[16]; for (int i = 0; i < 16; ++i) b[i] = 'Z';
	  sprintf(b, "ab%d", 1);        expect("sprintf NUL", b, "ab1"); }
	{ char b[16]; for (int i = 0; i < 16; ++i) b[i] = 'Z';
	  snprintf(b, 4, "abcdef");     expect("snprintf limit-1 + NUL", b, "abc");
	  b[0] = 'Q'; snprintf(b, 0, "x"); expect("snprintf limit 0 untouched", b, "Qbc"); }
	// LP64: %p は pointer 幅で読む (既定幅 = sizeof(void*)*2 桁)
	{ char b[64]; sprintf(b, "%p", (void*)(sizeof(void*) == 8 ? (unsigned long long)0x123456789abcull : 0x89abcull));
	  expect("%p full width", b, sizeof(void*) == 8 ? "0x0000123456789abc" : "0x00089abc"); }
	// %f: 正確変換 + 最近接偶数丸め (glibc printf と同値) / field width
	{ char b[64];
	  sprintf(b, "%.2f|%.1f|%.0f|%.0f|%.0f", 0.999, 9.96, 2.5, 3.5, -0.4);
	  expect("%f rounding", b, "1.00|10.0|2|4|-0");
	  sprintf(b, "%.15f|%f", 1e-10, 9007199254740993.0);
	  expect("%f tiny/large", b, "0.000000000100000|9007199254740992.000000");
	  sprintf(b, "[%8.3f][%-8.1f][%08.2f]", 3.14159, 2.25, -1.5);
	  expect("%f width", b, "[   3.142][2.2     ][-0001.50]"); }
	// %x: 最大桁 (8/16) を超える width は pad で埋める (従来は NUL が残り出力が切れた)
	{ char b[64]; sprintf(b, "[%10x][%-10x][%3x][%lld]", 0xab, 0xab, 0xabcd, -5LL);
	  expect("%x wide / %lld", b, "[  000000ab][000000ab  ][bcd][-5]"); }
	// 戻り値 = 文字数, 末尾 '\\' は終端を読み越さない
	{ char b[64]; int n = sprintf(b, "abc%d", 42);
	  expect("sprintf returns length", n == 5 ? "5" : "not 5", "5");
	  n = sprintf(b, "x\\"); expect("trailing backslash", b, "x"); (void)n; }
	// table (512) を越える長い %s は切り詰める (従来はスタックバッファ溢れ)
	{ static char big[700]; static char out[700];
	  for (int i = 0; i < 699; ++i) { big[i] = 'a'; }
	  big[699] = 0;
	  int n = sprintf(out, "%s", big);
	  expect("long %s truncated", (n == 511 && out[511] == 0) ? "ok" : "bad", "ok"); }
	return 0;
}
