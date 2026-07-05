#ifndef CORE_DBG_STUB_H
#define CORE_DBG_STUB_H
/*********************************************
 * core/include: デバッグマクロの no-op スタブ
 *
 * 旧コードは dbg_macro.h の「非 DEBUG 時」no-op 定義に依存していた。
 * 移植可能コアではデバッグ計装をすべて無効化する。各定義は #ifndef ガード付き
 * なので、ファイル側が独自定義していても衝突しない。
 *********************************************/

#ifndef DBG
#define DBG(...)
#endif
#ifndef DBGD
#define DBGD(...)
#endif
#ifndef DBGX
#define DBGX(...)
#endif
#ifndef DBGP
#define DBGP(...)
#endif
#ifndef DBGF
#define DBGF(...)
#endif
#ifndef DBGS
#define DBGS(...)
#endif
#ifndef DBGC
#define DBGC(...)
#endif
#ifndef DBGLD
#define DBGLD(...)
#endif
#ifndef DBGLX
#define DBGLX(...)
#endif
#ifndef DBGN
#define DBGN(...)
#endif
#ifndef DN
#define DN(...)
#endif
#ifndef DA
#define DA(...)
#endif
#ifndef DE
#define DE(...)
#endif
#ifndef DBGDN
#define DBGDN(...)
#endif
#ifndef DBGDN2
#define DBGDN2(...)
#endif
#ifndef DBGXN2
#define DBGXN2(...)
#endif
#ifndef DBG_BUF
#define DBG_BUF(...)
#endif
#ifndef DBG_PUTS
#define DBG_PUTS(...)
#endif
#ifndef DBG_BITD
#define DBG_BITD(...)
#endif
#ifndef DBG_DTOA
#define DBG_DTOA(...)
#endif
#ifndef DBG_DTOA_D
#define DBG_DTOA_D(...)
#endif
#ifndef DBG_LNTOADV_D
#define DBG_LNTOADV_D(...)
#endif
#ifndef DBG_FPRINTD
#define DBG_FPRINTD(...)
#endif
#ifndef DBG_PTAB
#define DBG_PTAB(...)
#endif
#ifndef DBG_PTAB_SUB
#define DBG_PTAB_SUB(...)
#endif
#ifndef FUNC_IN
#define FUNC_IN(...)
#endif
#ifndef FUNC_OUT
#define FUNC_OUT(...)
#endif
#ifndef SLIGHTIN
#define SLIGHTIN(...)
#endif
#ifndef SLIGHTOUT
#define SLIGHTOUT(...)
#endif
#ifndef print_array
#define print_array(...)
#endif

#endif /* CORE_DBG_STUB_H */
