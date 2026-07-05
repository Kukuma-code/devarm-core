#ifndef HAL_TIMER_H
#define HAL_TIMER_H
/*********************************************
 * HAL: 自由走行タイマの継ぎ目 (移植可能な契約)
 *
 * ハードウェア・番地・アセンブラを一切含まない。各プラットフォームが
 * base_tick を実装し、libc 側 (core/util/stimer 等) は get_tick を使う。
 *   - platform/host    : 単調クロック (clock_gettime) を ms tick に
 *   - platform/lpc2388 : タイマ0 カウンタ tmr0 (obsolete)
 * 旧 stimer が直読みしていた tmr0_reg.cnt / tmr0->get_cnt() をこの 1 継ぎ目に集約。
 *
 * tick は 32bit・自由走行で 2^32 を跨いでラップする前提 (stimer のラップ演算がこれに依存)。
 *********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* プラットフォームが提供する 32bit 自由走行カウンタ */
extern unsigned int (*base_tick)();

/* libc 側が使う安定名 (実装は hal/timer.cpp) */
unsigned int get_tick(void);

#ifdef __cplusplus
}
#endif
#endif /* HAL_TIMER_H */
