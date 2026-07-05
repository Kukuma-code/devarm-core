/*********************************************
 * core/util: ソフトタイマ実装 (get_tick 継ぎ目経由)
 *
 * 原典 armpp firmware/stimer.c からの変更:
 *   - tmr0_reg.cnt / tmr0->get_cnt() の HW 直読みを HAL の get_tick() に置換。
 *   - iselapsed() が原典で tmr0_reg.cnt を 2 回読んでいた (レース) のを 1 回読みに整理。
 *   - tick は 32bit (unsigned int)。ラップ (2^32 跨ぎ) 判定ロジックは原典どおり。
 *********************************************/
#include "stimer.h"
#include "core_config.h"   /* TRUE / FALSE */
#include "hal/timer.h"     /* get_tick */

stimer_::stimer_() : begin(0), cnt(0) {}
stimer_::~stimer_() {}

void stimer_::init()  { this->begin = 0; this->cnt = 0; }
void stimer_::reset() { this->init(); }

void stimer_::set_timer(unsigned int _cnt)
{
	unsigned int tmp = get_tick();
	this->begin = tmp;
	this->cnt   = tmp + _cnt;   /* 期間を足すと 2^32 を跨いでラップしうる */
}

int stimer_::istimer()
{
	unsigned int _cnt = get_tick();
	if (this->cnt < this->begin) {                 /* 期限がラップ済み (begin より下) */
		if (this->begin <= _cnt || _cnt <= this->cnt) return FALSE;
	} else {                                        /* 通常 */
		if (_cnt <= this->cnt) return FALSE;
	}
	return TRUE;
}

unsigned int stimer_::iselapsed()
{
	unsigned int tmp = get_tick();
	if (this->cnt < this->begin)
		if (tmp >= this->begin) return (0xffffffffu - tmp + 1u + this->cnt);
	return (this->cnt - tmp);
}
