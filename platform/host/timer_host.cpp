/*********************************************
 * platform/host: HAL タイマのホスト実装 (単調クロック -> 32bit ms tick)
 *
 * これにより stimer 等を macOS/Linux 上でビルド・テストできる。
 * LPC2388 実装は platform/lpc2388 が同じ base_tick を tmr0 で提供する (obsolete)。
 *********************************************/
#include "hal/timer.h"
#include <time.h>   /* clock_gettime, CLOCK_MONOTONIC */

static unsigned int host_tick()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	/* ミリ秒に丸め、32bit へ切り詰め (自由走行・ラップ相当) */
	unsigned long long ms = (unsigned long long)ts.tv_sec * 1000ull
	                      + (unsigned long long)ts.tv_nsec / 1000000ull;
	return (unsigned int)ms;
}

/* HAL の継ぎ目にホスト実装を差し込む */
unsigned int (*base_tick)() = host_tick;
