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
	return 0;
}
