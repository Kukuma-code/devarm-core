#ifndef CORE_IO_GETTEXT_H
#define CORE_IO_GETTEXT_H
/*********************************************
 * core/io: 行/語の入力ヘルパ (getchar 経由・移植可能)
 *
 * 旧 armpp gettext.{c,h} を抽出。名前は "gettext" だが GNU i18n とは無関係で、
 * 実体は「1 行読み込み」と「空白区切りの語の取り出し」:
 *   - get_line         : getchar() から 1 行を読む (HAL の入力継ぎ目 base_in に依存)
 *   - get_word         : バッファから空白区切りの 1 語を tobuf へコピー (純粋・HW 非依存)
 *   - get_word_forward : get_word + 区切りを 1 つ進めた次位置ポインタを返す (純粋)
 * 出力側 aout.h (put*) の入力側カウンターパート。get_line のみ getchar に依存する。
 * 関数名・引数順は原典を維持 (get_* は既に説明的で libc 名と非衝突・rename 対象外)。
 *********************************************/

/* getchar() から '\r'(または EOF) までを buf[0..len-1] に読み込み、NUL 終端する。
 * バックスペース(0x08)で 1 文字戻し、制御文字(< ' ')は取り込まない。 */
void get_line(unsigned char* buf, int len);

/* buf 先頭の空白区切り 1 語 (<=0x20 まで) を tobuf へコピーし NUL 終端する。 */
void get_word(unsigned char* buf, unsigned char* tobuf, int len);

/* get_word と同じだが、区切りを 1 つ越えた buf の次位置を返す (連続語の切り出し用)。 */
unsigned char* get_word_forward(unsigned char* buf, unsigned char* tobuf, int len);

#endif /* CORE_IO_GETTEXT_H */
