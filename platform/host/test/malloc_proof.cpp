/*********************************************
 * platform/host/test: C 割当 API malloc/free/realloc の回帰ハーネス (host)
 * core/mem/malloc.cpp が malloc/free/realloc を region allocator (mallocator) へ
 * 配線することを検証する:
 *   - malloc / realloc(NULL,n) が返すポインタが [heap_begin, heap_end) 内・書込み可
 *   - realloc の grow/shrink で既存バイトが保存される (min(old,new) を byte_copy)
 *   - malloc(0)==NULL / free(NULL) は no-op / realloc(p,0)==NULL(解放)
 * (printf は core/io の C++ linkage 版。<cstdio> は使わない)
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert -I core/mem -I core/block -I . \
 *     platform/host/test/malloc_proof.cpp core/mem/malloc.cpp core/mem/mallocator.cpp \
 *     core/convert/(all).cpp core/io/printf.cpp hal/io.cpp \
 *     platform/host/io_host.cpp platform/host/heap_host.cpp -o malloc_proof
 *********************************************/
#include <cstddef>
#include <cstdlib>   /* malloc/free/realloc の標準宣言（実体は core/mem/malloc.cpp が提供） */
#include "mallocator.h"
#include "hal/heap.h"

/* clang は -O1 以上で malloc/realloc を組込み関数として扱い、「malloc(0)==NULL」等の
   比較を定数畳み込みする (標準 malloc の意味論を仮定)。core の実装そのものを
   検証するため、volatile 関数ポインタ経由で呼んで畳み込みを止める。 */
static void* (*volatile p_malloc)(size_t) = malloc;
static void* (*volatile p_realloc)(void*, size_t) = realloc;
#define malloc(n) p_malloc(n)
#define realloc(p, n) p_realloc(p, n)

static int failed = 0;
static bool in_heap(void* p){ return (unsigned char*)p >= heap_begin && (unsigned char*)p < heap_end; }
static void check(const char* name, bool ok){
	printf("[%s] %s\n", ok ? "OK" : "FAIL", name);
	if (!ok) ++failed;
}

int main(){
	mallocator.init();

	// malloc / free
	char* p = (char*)malloc(64);
	check("malloc in heap", in_heap(p));
	bool writable = true;
	for (int i = 0; i < 64; ++i) p[i] = (char)i;
	for (int i = 0; i < 64; ++i) if (p[i] != (char)i) writable = false;
	check("malloc writable", writable);
	free(p);

	// realloc grow: 先頭 16 バイトが保存される
	unsigned char* a = (unsigned char*)malloc(16);
	for (int i = 0; i < 16; ++i) a[i] = (unsigned char)(i + 1);
	unsigned char* b = (unsigned char*)realloc(a, 64);
	check("realloc grow in heap", in_heap(b));
	bool keep = true;
	for (int i = 0; i < 16; ++i) if (b[i] != (unsigned char)(i + 1)) keep = false;
	check("realloc grow preserves bytes", keep);

	// realloc shrink: 先頭 8 バイトが保存される
	unsigned char* c = (unsigned char*)realloc(b, 8);
	bool keep2 = true;
	for (int i = 0; i < 8; ++i) if (c[i] != (unsigned char)(i + 1)) keep2 = false;
	check("realloc shrink preserves bytes", keep2);
	free(c);

	// 端条件
	check("malloc(0) == NULL", malloc(0) == 0);
	free(0);   // no-op (dealloc の下限判定)
	void* r = realloc(0, 32);
	check("realloc(NULL,n) == malloc", in_heap(r));
	free(r);
	check("realloc(p,0) == NULL", realloc(malloc(8), 0) == 0);

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
