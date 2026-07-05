#ifndef CORE_MEM_RAW_H
#define CORE_MEM_RAW_H
/*********************************************
 * core/mem: 生メモリ操作プリミティブ (移植可能・HW非依存・検査なし)
 *
 * 旧 include/mem.h のうち、printf にも mcheck (HW アクセス検査) にも依存
 * しない純粋な操作だけを core へ切り出したもの。ロジックは原典と同一。
 *
 * 命名は現行の terse 名を維持 (rename は最終パスで一括実施):
 *   mem_fill  = memset (val で dst を cnt バイト埋める)
 *   mem_compare  = memcmp (先頭 cnt バイトを比較。ret に差分/0 を返す out 引数版)
 *   store8/store16/store16 = 8/16/32bit の型付きストア (checked checked_store の下請け)
 * 引数順は原典の (src, dst) を維持 (標準 libc の (dst, src) とは逆)。
 *
 * core/block/byte.h の byte_fill/byte_copy とは別モジュール (旧 b vs mem) のため
 * 意図的に併存させる。mem_fill は (int val, int cnt)、byte_fill は
 * (unsigned char val, unsigned int cnt) と署名が異なる。rename 最終パスで
 * 統廃合を検討する。
 *
 * 関連:
 *   checked_store/dump_word/dump_range/dump8/dump16/dump32 … mcheck (HW アクセス検査) 依存の検査/
 *     ダンプ系。hal/mcheck.h の継ぎ目経由で core/mem/inspect.h に移送済み。
 *
 * 見送り (別スライス):
 *   mallocator/new/delete/realloc/mbound … sbrk syscall 依存のヒープ確保。
 *********************************************/
#include "core_config.h"   /* INLINE */

/* 埋める (memset) : dst を val で cnt バイト */
INLINE void mem_fill(int val, void* dst, int cnt){
	while (cnt--){
		*((char*)dst) = (char)val;
		dst = (void*)((unsigned char*)dst + 1);
	}
}

/* 比較 (memcmp) : 先頭 cnt バイト。全一致で ret=0、差異バイトで ret=差分 */
INLINE void mem_compare(const void* src, const void* dst, unsigned int cnt, int& ret){
	ret = 0;
	while (1){
		if (ret >= (int)cnt){ --ret; break; }
		if (((const unsigned char*)src)[ret] != ((const unsigned char*)dst)[ret]) break;
		++ret;
	}
	ret = ((const unsigned char*)src)[ret] - ((const unsigned char*)dst)[ret];
}

/* 型付きストア (8/16/32bit) : checked な checked_store の下請け。検査は行わない */
INLINE void store8(unsigned char  val, unsigned char*  addr){ *addr = val; }
INLINE void store16(unsigned short val, unsigned short* addr){ *addr = val; }
INLINE void store32(unsigned int   val, unsigned int*   addr){ *addr = val; }

#endif /* CORE_MEM_RAW_H */
