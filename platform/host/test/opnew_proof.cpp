/*********************************************
 * platform/host/test: グローバル operator new/delete の回帰ハーネス (host)
 * core/mem/operator_new.cpp が new/new[]/delete/delete[] を region allocator
 * (mallocator) へ配線することを検証する:
 *   - new / new[] / new(class) が返すポインタが [heap_begin, heap_end) 内
 *   - 値・コンストラクタが正しく作用し、配列が書込み可能
 *   - delete / delete[] 後に領域が再利用される
 * (printf は core/io の C++ linkage 版。<cstdio> は使わない)
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert -I core/mem -I . \
 *     platform/host/test/opnew_proof.cpp \
 *     core/mem/operator_new.cpp core/mem/mallocator.cpp core/convert/(all).cpp \
 *     core/io/printf.cpp hal/io.cpp platform/host/io_host.cpp platform/host/heap_host.cpp \
 *     -o opnew_proof
 *********************************************/
#include "mallocator.h"
#include "hal/heap.h"

struct Foo { int a; int b; Foo(int x):a(x),b(x*2){} };

static int failed = 0;
static bool in_heap(void* p){ return (unsigned char*)p >= heap_begin && (unsigned char*)p < heap_end; }
static void check(const char* name, bool ok){
	printf("[%s] %s\n", ok ? "OK" : "FAIL", name);
	if (!ok) ++failed;
}

int main(){
	mallocator.init();   // 継ぎ目確定後にクリーン状態から

	// new / delete (スカラ)
	int* p = new int(42);
	check("new int in heap", in_heap(p));
	check("new int value", *p == 42);
	delete p;

	// new[] / delete[]: 領域内・書込み可能
	int* arr = new int[16];
	check("new[] in heap", in_heap(arr));
	bool writable = true;
	for (int i = 0; i < 16; ++i) arr[i] = i;
	for (int i = 0; i < 16; ++i) if (arr[i] != i) writable = false;
	check("new[] writable", writable);
	delete[] arr;

	// new (クラス): コンストラクタ作用
	Foo* f = new Foo(7);
	check("new class in heap", in_heap(f));
	check("new class ctor ran", f->a == 7 && f->b == 14);
	delete f;

	// delete 後に領域が再利用される
	void* q = new char[8];
	check("new after free reuses region", in_heap(q));
	delete[] (char*)q;

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
