#ifndef CORE_IO_AOUT_H
#define CORE_IO_AOUT_H
/*********************************************
 * core/io: ASCII 出力 (putchar 経由・移植可能)
 *
 * 旧 include/a.h の puts / put* / print_bit を抽出。数値整形は conv に依存し、
 * 出力は HAL の putchar に依存する (HW 非依存)。printf 実装は core/io 配下。
 * 命名・引数順は原典を維持 (rename は最終パス)。
 *********************************************/
#include "core_config.h"
#include "hal/io.h"
#include "conv.h"

/* 文字列出力 (原典 puts: 改行は付けない) */
inline void puts(const char* src){
	unsigned int i = 0;
	while (src[i]) putchar(src[i++]);
}

/* 数値出力: n=10進 h=16進 b=2進 / u=unsigned l=long long / d=double
   一時バッファに conv で整形してから puts する。 */
inline void putn (int _num, int _width=0, int _pad=0x20){char t[0x20]; ntoad (_num,(char*)&t,_width,_pad); puts(t);}
inline void puth (int _num, int _width=8){char t[0xc];  ntoah (_num,(char*)&t,_width); puts(t);}
inline void putb (int _num, int _width=0x20){char t[0x24]; ntoab (_num,(char*)&t,_width); puts(t);}
inline void putun(unsigned int _num, int _width=0, int _pad=0x20){char t[0x20]; untoad(_num,(char*)&t,_width,_pad); puts(t);}
inline void putuh(unsigned int _num, int _width=8){char t[0xc]; untoah(_num,(char*)&t,_width); puts(t);}
inline void putub(unsigned int _num, int _width=0x20){char t[0x24]; untoab(_num,(char*)&t,_width); puts(t);}
inline void putln(long long _num, int _width=0, int _pad=0x20){char t[0x40]; lntoad(_num,(char*)&t,_width,_pad); puts(t);}
inline void putlh(long long _num, int _width=0x10){char t[0x14]; lntoah(_num,(char*)&t,_width); puts(t);}
inline void putlb(long long _num, int _width=0x40){char t[0x44]; lntoab(_num,(char*)&t,_width); puts(t);}
inline void putuln(unsigned long long _num, int _width=0, int _pad=0x20){char t[0x40]; ulntoad(_num,(char*)&t,_width,_pad); puts(t);}
inline void putulh(unsigned long long _num, int _width=0x10){char t[0x14]; ulntoah(_num,(char*)&t,_width); puts(t);}
inline void putulb(unsigned long long _num, int _width=0x40){char t[0x44]; ulntoab(_num,(char*)&t,_width); puts(t);}
inline void putd (double _num, int _width=0, int _pad=0x20){char t[0x44]; dtoa (_num,(char*)&t,_width,_pad); puts(t);}

/* ビット列表示 */
inline void print_bitd(long long ref){int bit[DOUBLE_DIGIT];
	for (int i=0;i<DOUBLE_DIGIT;i++) bit[i]=(ref>>i)&1;
	for (int i=DOUBLE_DIGIT-1;i>=0;i--){putchar(bit[i]);if(i%4==0)putchar(' ');}putchar('\n');}
inline void print_bit(long ref){int bit[LONG_DIGIT];
	for (int i=0;i<=LONG_DIGIT-1;i++) bit[i]=(ref>>i)&1;
	for (int i=LONG_DIGIT-1;i>=0;i--){putchar(bit[i]);if(i%4==0)putchar(' ');}putchar('\n');}

/* printf は conv.h が宣言済み (C++ linkage)。sprintf 系のみここで宣言。 */
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, unsigned int limit, const char* format, ...);

#endif /* CORE_IO_AOUT_H */
