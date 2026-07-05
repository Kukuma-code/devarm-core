/*********************************************
 * platform/host: ヒープ領域のホスト実装 (静的バッファ)
 *
 * core/mem/mallocator.h の region allocator が使う [heap_begin, heap_end) を
 * 静的配列で供給する。ポインタは定数初期化 (配列先頭 + 定数) なので、
 * どの動的初期化 (mallocator のコンストラクタ等) よりも前に確定する。
 * LPC2388 実装は platform/lpc2388 がリンカ配置の RAM で同じ継ぎ目を提供する (obsolete)。
 *********************************************/
#include <cstddef>   /* std::max_align_t */
#include "hal/heap.h"

#ifndef HOST_HEAP_SIZE
#define HOST_HEAP_SIZE (64 * 1024)
#endif

/* heap_begin は allocator の整列境界 (MALLOCATOR_ALIGN, 既定 8) 以上に整列して
   いなければならない (hal/heap.h の契約)。unsigned char 配列は既定で
   alignof=1 のため、max_align_t 境界へ明示的に整列する (>=8 を常に満たす)。 */
alignas(std::max_align_t) static unsigned char host_heap[HOST_HEAP_SIZE];

extern "C" unsigned char* heap_begin = host_heap;
extern "C" unsigned char* heap_end   = host_heap + HOST_HEAP_SIZE;
