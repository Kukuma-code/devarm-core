#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H
/*********************************************
 * core/include: 移植可能コアの基本設定 (host ビルド可能)
 *
 * 旧 common.h / presettings.h を core 用に置き換えるもの。
 *   - ビルド時生成ヘッダ (dbg_log_define.h / dbg_define.h) に依存しない
 *   - arm.h (HW) を引かない
 *   - デバッグマクロ (DBG_*) は no-op
 * これにより conv / io などのコアを macOS/Linux 上でビルド・テストできる。
 *********************************************/

/* --- inline 制御 (旧 common.h と同義) --- */
#ifndef INLINE_DISABLE
#define INLINE inline __attribute__((flatten))
#else
#define INLINE
#endif
#ifdef INLINE_ENABLE
#define nINLINE inline __attribute__((flatten))
#else
#define nINLINE
#endif

/* --- 定数 --- */
#ifndef NULL
#define NULL 0
#endif
#ifndef NONE
#define NONE 0
#endif
#define TRUE 1
#define FALSE 0
#define LONG_DIGIT 0x20
#define DOUBLE_DIGIT 0x40

/* size_ は「define で型宣言」という原典の規約 (cpp_rules)。
   標準 size_t とは衝突させないため独自名のみ定義。 */
#ifndef size_
#define size_ unsigned int
#endif

/* qty_ は個数/数量を表す符号付き型。原典 types.h の
   `typedef long rqty_; typedef rqty_ qty_;` を define 規約で畳んだもの
   (read() 等の戻り値・要素数に使う。elf/welf 等が依存)。 */
#ifndef qty_
#define qty_ long
#endif

/* fd 定数 (旧 presettings.h) */
#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* 数字判定 (旧 common.h。printf_core 内で局所変数 ch に対して使う) */
#define _DIGIT_ ((ch>='0')&&(ch<='9'))

/* 結果コード: 原典の result.h は NUL_(x) を 0 に潰す (生成物 result_define.h 不要) */
#ifndef NUL_
#define NUL_(x) 0
#endif

/* --- デバッグマクロは移植可能コアでは no-op (dbg_stub.h) --- */
/* DBG_DTOA_ENABLE は未定義のまま = デバッグ無効 */
#include "dbg_stub.h"

#endif /* CORE_CONFIG_H */
