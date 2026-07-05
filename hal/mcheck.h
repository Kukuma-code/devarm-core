#ifndef HAL_MCHECK_H
#define HAL_MCHECK_H
/*********************************************
 * HAL: メモリアクセス可否チェックの継ぎ目 (移植可能な契約)
 *
 * ハードウェア・番地レンジを一切含まない。各プラットフォームが
 * mem_can_read / mem_can_write を実装し、上位 (core/mem/inspect.h の
 * checked_store/dump_word/dump_range) はその継ぎ目だけを見る。
 *   - platform/host    : フラットアドレス空間 (常に許可)
 *   - platform/lpc2388 : LPC2388 のメモリマップ番地レンジ (obsolete)
 * 旧 arm.h / platform/lpc2388/mcheck.h の HW 依存部を継ぎ目化したもの。
 *
 * 命名は現行の terse 名を維持 (rename は最終パスで一括実施)。
 *********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* プラットフォームが提供する可否判定 (1=可 / 0=不可) */
extern int (*mem_can_read)(const void* addr);
extern int (*mem_can_write)(const void* addr);

#ifdef __cplusplus
}

/* 上位が使う互換アクセサ (旧 arm.h と同名・同義)。継ぎ目を呼ぶだけ。 */
inline int mcheck_read(const void* addr){ return mem_can_read(addr); }
inline int mcheck_write(const void* addr){ return mem_can_write(addr); }
#endif

#endif /* HAL_MCHECK_H */
