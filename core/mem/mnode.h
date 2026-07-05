#ifndef CORE_MEM_MNODE_H
#define CORE_MEM_MNODE_H
/*********************************************
 * core/mem: allocator ノードヘッダ (移植可能・最小構成)
 *
 * 旧 include/mnode.h のうち region allocator の alloc/dealloc が実際に使う
 * 部分だけを抽出したもの: ノードヘッダ {size, state} と状態フラグ enum。
 *
 * 見送り (別スライス / 原典 mnode.h に残置):
 *   refcnt (state 下位バイトの参照カウント union) と CLASS_ARRAY フラグ、
 *   mnode_get/get_num/info など。これらは new[]/参照カウント/prn ストリーム
 *   出力に必要で、prn.h・result.h 等の別サブシステムに依存する。
 *   本スライスの region allocator 中核は state を丸ごと int として扱うだけで
 *   足りるため、union を畳んで単純な {size, state} にしている。
 *********************************************/
#include <cstdint>   /* uintptr_t (ポインタ幅整数演算) */

namespace MALLOCATOR {
enum {
	CLEAN       = 0,
	REFCNT      = 0x000000ff,   /* 参照カウント (本スライスでは未使用・enum は保持) */
	HAS_SPACE   = 0x20000000,
	USED        = 0x40000000,
	CLASS_ARRAY = 0x80000000,   /* new[] 用 (本スライスでは未使用) */
};
}

#define MNODE mnode_
/* ヘッダは {size, state} の 2 語。原典は __attribute__((packed)) を付けていたが
   本 allocator はノードを自前で MALLOCATOR_ALIGN 境界に配置するため packed は
   不要。むしろ packed はヘッダを未整列とみなさせ、ARM 等で size/state への
   バイト単位アクセスを誘発して逆効果になるため外した (alignof=4, sizeof=8 は不変)。
   注意: sizeof(MNODE) は MALLOCATOR_ALIGN の倍数でなければならない
   (ヘッダ直後の user ポインタ = node + sizeof(MNODE) を整列させるため)。
   mallocator.h 側で static_assert により保証する。 */
struct mnode_ {
	unsigned int size;    /* ペイロードのバイト数 (ヘッダ除く) */
	unsigned int state;   /* MALLOCATOR フラグ + 下位バイトに refcnt */
};

/* 次ノードへ (ヘッダ + ペイロードを跨ぐ)。ポインタ幅で計算する */
#define NEXT_MNODE(x) (x) = (MNODE*)((uintptr_t)(x) + (x)->size + sizeof(MNODE))

#endif /* CORE_MEM_MNODE_H */
