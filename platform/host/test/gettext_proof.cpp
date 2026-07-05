/*********************************************
 * platform/host/test: core/io/gettext の回帰ハーネス (host)
 * get_word / get_word_forward (純粋) と get_line (getchar 経由) を検証する。
 * get_line は stdin から 1 行読むので、パイプ入力 "abc def\r" を与えて実行する:
 *   printf 'abc def\r' | ./gettext_proof
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert -I core/io -I . \
 *     platform/host/test/gettext_proof.cpp core/io/gettext.cpp \
 *     hal/io.cpp platform/host/io_host.cpp -o gettext_proof
 * (puts は aout.h の inline、putchar は hal/io.cpp。conv は未使用のためリンク不要)
 *********************************************/
#include "gettext.h"
#include "aout.h"   /* puts / putchar */

static int failed = 0;

static bool ueq(const unsigned char* a, const char* b){
	int i = 0;
	for (; a[i] && b[i]; ++i) if (a[i] != (unsigned char)b[i]) return false;
	return a[i] == 0 && b[i] == 0;
}
static void check(const char* name, bool ok){
	puts(ok ? "[OK] " : "[FAIL] "); puts(name); putchar('\n');
	if (!ok) ++failed;
}

int main(){
	unsigned char src[] = "hello world foo";
	unsigned char w[32];

	get_word(src, w, 31);
	check("get_word -> hello", ueq(w, "hello"));

	unsigned char* p = get_word_forward(src, w, 31);
	check("get_word_forward word -> hello", ueq(w, "hello"));
	get_word(p, w, 31);
	check("get_word_forward advance -> world", ueq(w, "world"));

	// get_line: stdin から "abc def\r" を読む想定 (空白は保持、'\r' で終端)
	unsigned char line[64];
	get_line(line, 64);
	check("get_line -> abc def", ueq(line, "abc def"));

	puts(failed ? "SOME FAILED\n" : "ALL PASS\n");
	return failed ? 1 : 0;
}
