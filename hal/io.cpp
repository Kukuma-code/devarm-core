/*********************************************
 * HAL: putchar / getchar の移植可能な実装
 * base_out / base_in はプラットフォームが定義する (platform 以下)。
 *********************************************/
#include "io.h"

extern "C" int putchar(int ch)
{
	if (ch == '\n')
		base_out('\r');   /* 端末向け CR+LF 変換 (旧 arm.h と同じ挙動) */
	base_out((char)ch);
	return ch;
}

extern "C" int getchar(void)
{
	return base_in ? base_in() : -1;
}
