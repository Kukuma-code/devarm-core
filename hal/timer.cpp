/*********************************************
 * HAL: 自由走行タイマの移植可能な実装
 * base_tick はプラットフォームが定義する (platform 以下)。
 *********************************************/
#include "hal/timer.h"

extern "C" unsigned int get_tick(void)
{
	return base_tick ? base_tick() : 0;
}
