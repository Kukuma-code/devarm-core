/*********************************************
 * platform/host: HAL 入出力のホスト実装 (stdout / stdin)
 *
 * これにより移植可能コアを macOS/Linux 上でビルド・テストできる。
 * LPC2388 実装は platform/lpc2388 が同じ base_out/base_in を UART で提供する (obsolete)。
 *********************************************/
#include "hal/io.h"
#include <unistd.h>   /* read, write */

static void host_out(char c)
{
	ssize_t n = write(1, &c, 1);   /* STDOUT_FILENO */
	(void)n;
}

static int host_in()
{
	unsigned char c;
	return read(0, &c, 1) == 1 ? (int)c : -1;   /* STDIN_FILENO */
}

/* HAL の継ぎ目にホスト実装を差し込む */
void (*base_out)(char) = host_out;
int  (*base_in)()      = host_in;
