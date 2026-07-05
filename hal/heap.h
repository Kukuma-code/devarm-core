#ifndef HAL_HEAP_H
#define HAL_HEAP_H
/*********************************************
 * HAL: ヒープ領域の継ぎ目 (移植可能な契約)
 *
 * ハードウェア・リンカスクリプトを一切含まない。各プラットフォームが
 * ヒープ領域 [heap_begin, heap_end) を提供し、core/mem の allocator は
 * この 2 ポインタだけを見る。
 *   - platform/host    : 静的バッファ (heap_host.cpp)
 *   - platform/lpc2388 : リンカ配置の RAM 領域 (obsolete)
 *
 * 旧コードは allocbegin / allocend という「リンカラベルの番地」(&allocbegin)
 * で領域を表していた (lib/testlib/testlib.c の inline asm)。これは host で
 * 再現しづらいため、素直な実行時ポインタ変数の継ぎ目へ置き換えたもの。
 * 命名は現行の terse 感を踏襲しつつ役割を明示 (rename は最終パスで一括)。
 *********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* プラットフォームが提供するヒープ領域 [heap_begin, heap_end)
 *
 * 契約: heap_begin は allocator の整列境界 (core/mem: MALLOCATOR_ALIGN, 既定 8)
 * 以上に整列していること。allocator はノードを heap_begin から境界の倍数で
 * 並べ、返却ポインタの整列を heap_begin の整列に帰着させるため、ここが未整列
 * だと全確保が未整列になる。host は alignas で保証 (heap_host.cpp)、実機は
 * リンカ配置を境界整列させること。 */
extern unsigned char* heap_begin;
extern unsigned char* heap_end;

#ifdef __cplusplus
}
#endif
#endif /* HAL_HEAP_H */
