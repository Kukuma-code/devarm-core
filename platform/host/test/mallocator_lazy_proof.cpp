/*********************************************
 * platform/host/test: mallocator の遅延初期化の回帰ハーネス (host)
 *
 * 不変条件:
 *   1) 動的初期化 (コンストラクタ) より前の確保要求にも応える。
 *      ELF (Linux) では core/mem/malloc.cpp の malloc が process 全体へ interpose され、
 *      libstdc++ の EH pool 初期化などがコンストラクタより先に malloc を呼ぶ。
 *      静的記憶域はゼロ初期化されるので、その時点の mallocator は全メンバ 0。
 *   2) その後に走るコンストラクタは、先行確保を孤児化しない (init し直さない)。
 *      し直すと frontier が heap_begin へ戻り、次の確保が先行ブロックと重なる。
 *
 * 本物の初期化順は TU 間で制御できないので、ゼロ初期化した静的記憶域に置いた
 * mallocator_ を「構築前」とみなし、確保 -> placement new -> 確保 の順で再現する。
 * global の mallocator (core/mem/mallocator.cpp) はリンクしない (heap を共有するため)。
 *********************************************/
#include <new>        /* placement new */
#include "mallocator.h"
#include "hal/heap.h"

static int failed = 0;
static bool in_heap(const void* p){
	return (const unsigned char*)p >= heap_begin && (const unsigned char*)p < heap_end;
}
static void check(const char* name, bool ok){
	printf("[%s] %s\n", ok ? "OK" : "FAIL", name);
	if (!ok) ++failed;
}

/* ゼロ初期化される静的記憶域 = 動的初期化前の global mallocator と同じ状態 */
alignas(mallocator_) static unsigned char storage[sizeof(mallocator_)];

int main(){
	mallocator_* m = reinterpret_cast<mallocator_*>(storage);

	/* 1) 構築前の確保 */
	unsigned char* early = m->try_alloc(32);
	check("pre-construction try_alloc returns heap memory", early != 0 && in_heap(early));
	for (int i = 0; i < 32 && early; ++i) early[i] = (unsigned char)(0xA0 + i);

	/* 2) 後から走るコンストラクタが先行確保を壊さない */
	new (storage) mallocator_();
	unsigned char* later = m->try_alloc(32);
	check("post-construction try_alloc returns heap memory", later != 0 && in_heap(later));
	bool disjoint = early && later && (later >= early + 32 || later + 32 <= early);
	check("constructor keeps the early block (no overlap)", disjoint);
	bool intact = early != 0;
	for (int i = 0; i < 32 && early; ++i) if (early[i] != (unsigned char)(0xA0 + i)) intact = false;
	check("early block contents survive construction", intact);

	/* operator new 経路 (alloc) も同じ遅延初期化を通る: 送出せず確保できる */
	for (unsigned i = 0; i < sizeof(storage); ++i) storage[i] = 0;
	bool threw = false;
	unsigned char* via_alloc = 0;
	try { via_alloc = m->alloc(16); } catch (const std::bad_alloc&) { threw = true; }
	check("pre-construction alloc (operator new path) does not throw", !threw && in_heap(via_alloc));

	printf(failed ? "FAILED %d\n" : "ALL OK\n", failed);
	return failed ? 1 : 0;
}
