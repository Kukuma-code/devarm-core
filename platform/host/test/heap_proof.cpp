/*********************************************
 * platform/host/test: devarm core/mem region allocator の回帰ハーネス (host)
 * core/mem/mallocator.h をネイティブビルドし、
 *   - 確保ブロックが領域内・非重複・書込み可能
 *   - 全解放で share=0 -> 先頭へリセット (dealloc の減算/リセット経路)
 *   - 領域を超える確保で std::bad_alloc
 *   - 使用量が容量の過半に達したときの空きノード再利用 (next_reuse_node)
 *   - 奇数サイズを混ぜても返却ポインタが MALLOCATOR_ALIGN 境界を保つ (整列対応)
 * を検証する。(printf は core/io の C++ linkage 版。<cstdio> は使わない)
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert \
 *     -I core/mem -I . platform/host/test/heap_proof.cpp \
 *     core/convert/(all).cpp core/io/printf.cpp core/mem/mallocator.cpp \
 *     hal/io.cpp platform/host/io_host.cpp platform/host/heap_host.cpp -o heap_proof
 *********************************************/
#include "mallocator.h"
#include "raw.h"   /* mem_fill */

static int failed = 0;
static void check(const char* name, bool ok){
	printf("[%s] %s\n", ok ? "OK" : "FAIL", name);
	if (!ok) ++failed;
}

int main(){
	mallocator.init();   /* 継ぎ目確定後にクリーン状態から開始 */

	// 基本確保: 領域内・非重複・書込み可能
	{
		const int N = 4;
		const unsigned int S = 64;
		unsigned char* p[N];
		bool in_range = true, distinct = true, writable = true;
		for (int i = 0; i < N; ++i){
			p[i] = mallocator.alloc(S);
			if (!(p[i] >= heap_begin && p[i] + S <= heap_end)) in_range = false;
			for (int k = 0; k < i; ++k)
				if (!(p[i] + S <= p[k] || p[k] + S <= p[i])) distinct = false;  // 非重複
		}
		for (int i = 0; i < N; ++i) mem_fill((char)('A' + i), p[i], S);   // 各ブロックを別値で埋める
		for (int i = 0; i < N; ++i)
			for (unsigned int j = 0; j < S; ++j)
				if (p[i][j] != (unsigned char)('A' + i)) writable = false;   // 相互汚染なし
		check("alloc in range", in_range);
		check("alloc distinct/non-overlap", distinct);
		check("alloc writable, no cross-contam", writable);

		for (int i = 0; i < N; ++i) mallocator.dealloc(p[i]);
	}

	// 全解放後は先頭へリセット: 次の確保が最初の user ポインタに戻る
	{
		unsigned char* a = mallocator.alloc(64);
		unsigned char* first = heap_begin + sizeof(MNODE);
		check("free-all resets to begin", a == first);
		mallocator.dealloc(a);
	}

	// 空きノード再利用: 使用量が容量の過半に達すると HAS_SPACE が立ち再利用される
	{
		const int N = 4;
		const unsigned int S = 200;   // 4*(S+hdr) > PAGE_SZ/2 になるよう十分大きく
		unsigned char* p[N];
		for (int i = 0; i < N; ++i) p[i] = mallocator.alloc(S);
		// 先頭 3 つを解放 (隣接だが間の used ブロックにより即併合はされない)
		mallocator.dealloc(p[0]);
		mallocator.dealloc(p[1]);
		mallocator.dealloc(p[2]);
		unsigned char* reused = mallocator.alloc(S);
		check("reuse hits a freed slot",
		      reused == p[0] || reused == p[1] || reused == p[2]);
		mallocator.init();
	}

	// 整列: 奇数サイズを混ぜても返却ポインタは MALLOCATOR_ALIGN 境界を保つ。
	// 分割 (split) 後の残ノード再利用も含め全ポインタを検査する。
	{
		mallocator.init();
		const unsigned int sizes[] = { 1, 3, 7, 13, 17, 31, 33, 65, 100, 129 };
		const int N = (int)(sizeof(sizes) / sizeof(sizes[0]));
		unsigned char* p[N];
		bool aligned = true;
		for (int i = 0; i < N; ++i){
			p[i] = mallocator.alloc(sizes[i]);
			if ((uintptr_t)p[i] % MALLOCATOR_ALIGN != 0) aligned = false;
		}
		check("odd-size allocs stay aligned", aligned);

		// 大きめブロックを解放 → 小要求で再利用/分割させ、残余ポインタも整列か
		mallocator.dealloc(p[N - 1]);
		unsigned char* q = mallocator.alloc(5);
		check("realigned after split stays aligned",
		      (uintptr_t)q % MALLOCATOR_ALIGN == 0);
		mallocator.init();
	}

	// 回帰 (bug_log id=99): 非分割 reuse で穴の全スパンが保持され、走査タイリングが
	// 崩れない。穴サイズ S に対し _size が [S-24, S] (= 非分割条件 S <= _size+hdr+16)
	// となる再利用で、原典逸脱版は hole->size を _size へ切り詰め端数を孤児化していた。
	{
		mallocator.init();
		// 単一ページ内 (合計 < PAGE_SZ) に収め HAS_SPACE を成立させる:
		//   diff(=frontier) > size/2 かつ share < diff/2。壁 B/E は USED のまま残す。
		unsigned char* pA = mallocator.alloc(256);   // 穴候補 (span 264)
		unsigned char* pB = mallocator.alloc(32);    // 壁: A の穴を後続併合から隔離
		unsigned char* pC = mallocator.alloc(256);   // 詰め物 (解放で share を減らす)
		unsigned char* pE = mallocator.alloc(32);    // 壁: frontier を保持
		(void)pE;
		mallocator.dealloc(pA);   // 穴 A (B が USED なので span 264 のまま)
		mallocator.dealloc(pC);   // share 縮小 -> HAS_SPACE 成立
		// 非分割 reuse: _size=240 は 256 <= 240+8+16=264 を満たし分割しない
		unsigned char* reused = mallocator.alloc(240);
		MNODE* rn = (MNODE*)(reused - sizeof(MNODE));
		check("non-split reuse hits the hole", reused == pA);
		// 走査タイリング不変条件: 再利用ノードの末尾 == 次ノード (B) のヘッダ。
		// 穴の全スパン (256) が保たれていれば一致、_size へ切り詰められると不一致。
		check("non-split reuse preserves hole span (scan tiling intact)",
		      (uintptr_t)reused + rn->size == (uintptr_t)pB - sizeof(MNODE));
		mallocator.init();
	}

	// 回帰 (bug_log id=100): dealloc は冪等。同一ポインタの二重解放が share を
	// アンダーフローさせず、全解放時の frontier リセット経路を殺さない。
	{
		mallocator.init();
		unsigned char* p = mallocator.alloc(64);    // span 72
		unsigned char* q = mallocator.alloc(128);   // span 136 (72 と非対称で偶発 share==0 を避ける)
		mallocator.dealloc(p);
		mallocator.dealloc(p);   // 二重解放: ガードが無いと share が壊れる
		mallocator.dealloc(q);   // これで全解放 -> 健全なら frontier が先頭へ戻る
		unsigned char* r = mallocator.alloc(16);
		check("double-free is idempotent (full-free still resets to begin)",
		      r == heap_begin + sizeof(MNODE));
		mallocator.init();
	}

	// 領域超過で std::bad_alloc
	{
		bool threw = false;
		try {
			mallocator.alloc(1024u * 1024u);   // host heap (64KB) を超える
		} catch (const std::bad_alloc&) {
			threw = true;
		}
		check("over-bound throws bad_alloc", threw);
		mallocator.init();
	}

	printf("%s\n", failed ? "SOME FAILED" : "ALL PASS");
	return failed ? 1 : 0;
}
