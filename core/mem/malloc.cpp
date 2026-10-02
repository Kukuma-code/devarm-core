/*********************************************
 * core/mem: C 割当 API malloc/free/realloc を region allocator へ配線
 *
 * 原典 lib/mem では malloc/free は `FUNC_ALIAS(malloc)`/`FUNC_ALIAS(free)` による
 * operator new/delete への inline-asm 別名（独立実装なし）。realloc のみ実体あり。
 * host/C++17 では inline asm を使わず、素の C linkage 関数として mallocator へ配線する。
 * bare-metal target（system libc 無し）ではこれが libc の malloc/free/realloc になる。
 *
 * 原典からの変更:
 *   - FUNC_ALIAS(inline asm) 廃止 → 直接 mallocator.alloc/dealloc を呼ぶ。
 *   - malloc は C 意味論に合わせ、確保失敗時に NULL を返す（operator new は送出、
 *     C malloc は NULL が正）。送出→捕捉ではなく非送出の mallocator.try_alloc を使う:
 *     例外オブジェクトの確保自体が malloc へ再入し無限再帰するため（mallocator.h 参照）。
 *   - realloc は旧ノードの payload サイズ（MNODE::size, alloc 時に MALLOCATOR_ALIGN へ
 *     丸め済み）を読み、min(old,new) を byte_copy でコピー。
 *
 * 別確保系 mbound/brk/sbrk（OS ヒープ拡張）は未移植: devarm は hal/heap の固定領域
 *   [heap_begin,heap_end) + enlarge で置換済みのため不要（brk/sbrk は host でも非推奨）。
 *
 * host 注意: これらはグローバル C シンボル。(malloc は macOS SDK が noexcept なしで宣言するため noexcept は付けない; 実際は内部捕捉で非送出。)
 *   macOS の二段階名前空間では libSystem 内部の
 *   malloc 使用とは分離されるが、フラット名前空間や firmware では process 全体を担う。
 *********************************************/
#include <cstddef>       /* size_t */
#include "mallocator.h"  /* mallocator / MNODE / size_ / uintptr_t */
#include "byte.h"        /* byte_copy (src, dst, cnt) */

extern "C" void* malloc(size_t size) {
	if ( size == 0 ) return 0;
	return (void*)mallocator.try_alloc((size_)size);   /* 非送出 (理由は try_alloc 参照) */
}

extern "C" void free(void* ptr) {
	mallocator.dealloc((unsigned char*)ptr);   /* NULL は dealloc の下限判定で no-op */
}

extern "C" void* realloc(void* ptr, size_t size) {
	if ( ptr == 0 ) return malloc(size);          /* realloc(NULL,n) == malloc(n) */
	if ( size == 0 ) { free(ptr); return 0; }     /* realloc(p,0): 解放して NULL */
	MNODE* node = (MNODE*)((uintptr_t)ptr - sizeof(MNODE));
	size_ old = node->size;                        /* payload バイト数 (丸め済み) */
	void* nw = malloc(size);
	if ( nw == 0 ) return 0;                        /* 失敗時は元 ptr を保持 (C 準拠) */
	size_ n = (old < (size_)size) ? old : (size_)size;
	byte_copy((const void*)ptr, nw, n);
	free(ptr);
	return nw;
}
