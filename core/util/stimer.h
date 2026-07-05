#ifndef CORE_UTIL_STIMER_H
#define CORE_UTIL_STIMER_H
/*********************************************
 * core/util: ソフトタイマ (自由走行 tick の上に載る経過/期限判定)
 *
 * 旧 armpp firmware/stimer.{c,h} を抽出。HW タイマ tmr0 の直読みを HAL の
 * get_tick() 継ぎ目 (hal/timer.h) に置換した。ラップ (2^32 跨ぎ) を含む
 * 期限判定 istimer() / 残り時間 iselapsed() のロジックはそのまま。
 *
 * 原典からの変更:
 *   - tick 型を unsigned long -> unsigned int (32bit 固定)。原典のラップ演算
 *     (0xffffffff) は 32bit 前提で、LP64 host の 64bit long では壊れるため。
 *   - firmware 固有の friend var_set / グローバル実体 stimer は移植せず、
 *     再利用可能なクラスのみを残す。
 *********************************************/

class stimer_ {
 public:
	unsigned int begin;   /* set_timer 時の tick */
	unsigned int cnt;     /* 期限 tick (begin + 期間、ラップしうる) */
 public:
	stimer_();
	~stimer_();
	void init();
	void reset();
	void set_timer(unsigned int _cnt);
	int  istimer();        /* 期限を過ぎたら TRUE (ラップ対応) */
	unsigned int iselapsed();
};

#endif /* CORE_UTIL_STIMER_H */
