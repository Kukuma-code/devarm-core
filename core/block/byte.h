#ifndef CORE_BLOCK_BYTE_H
#define CORE_BLOCK_BYTE_H
/*********************************************
 * core/block: バイトブロックプリミティブ (移植可能・HW非依存)
 *
 * 旧 b モジュール (バイト単位のブロック操作) を core へ切り出したもの。
 *   byte_copy=memcpy, byte_fill=memset
 * mem モジュール (mem_fill/mem_compare/checked_store/dump_word) は mcheck による HW アクセス検査を
 * 伴うため別物。ここは検査なしの純粋なバイト操作のみを置く。
 *
 * 命名は現行の terse 名を維持 (rename は最終パスで一括実施)。
 * 引数順は原典の (src, dst) を維持 (標準 libc の (dst, src) とは逆)。
 *
 * 出所:
 *   byte_copy … 旧 include/b.h の inline (および lib/b/byte_copy.c) を忠実に整形。
 *          ロジックは原典と同一。
 *   byte_fill … 旧 inc/b/byte_fill.inc (ARM asm) のセマンティクス (dst を val で cnt
 *          バイト埋める) から移植可能 inline として再構成。標準 memset 相当。
 *
 * 見送り:
 *   bzero… 旧 inc/b/bzero.inc は asm のみ (C ソース・テストなし)。名前が
 *          POSIX <strings.h> の bzero と衝突し core の host ビルドを壊す上、
 *          byte_fill(0, dst, cnt) と等価で追加価値がない。final rename パスで
 *          衝突しない名前へ寄せる際に再検討する。
 *   asm 版は LPC2388/ARM 固有 (obsolete) のため移送せず inc/b/ に隔離のまま。
 *********************************************/

/* バイトコピー (memcpy) : src -> dst, cnt バイト */
inline void byte_copy(const void* src, void* dst, unsigned int cnt){
	for (unsigned int i = 0; i < cnt; ++i)
		((unsigned char*)dst)[i] = ((const unsigned char*)src)[i];
}

/* バイト埋め (memset) : dst を val で cnt バイト */
inline void byte_fill(unsigned char val, void* dst, unsigned int cnt){
	for (unsigned int i = 0; i < cnt; ++i)
		((unsigned char*)dst)[i] = val;
}

#endif /* CORE_BLOCK_BYTE_H */
