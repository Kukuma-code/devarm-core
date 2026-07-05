#ifndef HAL_IO_H
#define HAL_IO_H
/*********************************************
 * HAL: 文字入出力の継ぎ目 (移植可能な契約)
 *
 * ハードウェア・番地・アセンブラを一切含まない。各プラットフォームが
 * base_out / base_in を実装し、libc 側は putchar / getchar を使う。
 *   - platform/host    : stdout / stdin
 *   - platform/lpc2388 : UART (obsolete)
 * 旧 arm.h から HAL 部分のみを抽出したもの。
 *********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* プラットフォームが提供する 1 文字入出力 */
extern void (*base_out)(char);   /* 1 文字出力 */
extern int  (*base_in)();        /* 1 文字入力 (無ければ -1) */

/* libc 側が使う標準入出力 (実装は hal/io.cpp) */
int putchar(int ch);             /* '\n' -> CR+LF に変換して base_out */
int getchar(void);

#ifdef __cplusplus
}
#endif
#endif /* HAL_IO_H */
