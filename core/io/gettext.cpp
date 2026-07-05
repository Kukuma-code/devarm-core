/*********************************************
 * core/io: 行/語の入力ヘルパ実装 (getchar 経由)
 *
 * 原典 armpp gettext.c からの変更:
 *   - 依存 include を整理: common.h / a.h を撤去 (どちらも本ファイルでは未使用)。
 *     get_line が使う getchar() は HAL の入力継ぎ目 hal/io.h が宣言する。
 *   - get_line: getchar() の戻り値を int で受け、EOF(<0) でも終端する
 *     (firmware のブロッキング getchar は -1 を返さないため挙動不変。host の
 *      パイプ/リダイレクト入力で無限ループしないための可搬化)。原典どおり終端は '\r'。
 *   - get_word_forward: 原典の `*buf++;` (デリファレンス結果は破棄) を、意図どおりの
 *     `++buf;` (区切りを 1 つ進める) に整理。挙動は同一。
 *********************************************/
#include "gettext.h"
#include "hal/io.h"   /* getchar (base_in 継ぎ目) */

void get_line(unsigned char* buf, int len)
{
	int idx = 0;
	for (;;) {
		int c = getchar();
		if (c < 0 || c == '\r') break;          /* EOF もしくは復帰で終端 */
		if (c == '\b') { if (idx) --idx; continue; }
		if (c >= ' ' && idx < len - 1) buf[idx++] = (unsigned char)c;
	}
	buf[idx] = 0;
}

void get_word(unsigned char* buf, unsigned char* tobuf, int len)
{
	for (int idx = 0; idx < len; idx++) {
		if (*buf <= 0x20) break;                /* 空白/制御で語の終わり */
		*tobuf++ = *buf++;
	}
	*tobuf = 0;
}

unsigned char* get_word_forward(unsigned char* buf, unsigned char* tobuf, int len)
{
	for (int idx = 0; idx < len; idx++) {
		if (*buf <= 0x20) break;
		*tobuf++ = *buf++;
	}
	*tobuf = 0;
	++buf;                                       /* 区切りを 1 つ越えて次語の先頭へ */
	return buf;
}
