#ifndef CORE_MEM_INSPECT_H
#define CORE_MEM_INSPECT_H
/*********************************************
 * core/mem: メモリ検査/ダンプ (mcheck 継ぎ目 + printf 経由・移植可能)
 *
 * 旧 include/mem.h のうち mcheck (HW アクセス検査) に依存する部分を、
 * hal/mcheck.h の継ぎ目経由に置き換えて移植可能にしたもの。ロジックは
 * 原典と同一 (下記のポインタ出力キャストの host 対応を除く)。
 *   checked_store   … 可否検査つき型付きストア (mcheck_write ガード + トレース出力)
 *   dump_word  … 可否検査つき 1 語ダンプ (8/16/32bit)
 *   dump_range … 可否検査つき連続ダンプ (16 進, _sep_sz 区切り)
 *   dump8/dump16/dump16 … 検査なしの型別ダンプ (dump_word の下請け)
 *
 * 出力は core/io の printf (conv.h が宣言) と HAL の putchar に依存し、
 * アクセス可否は HAL の mcheck 継ぎ目に依存する。純粋な set/cmp/store は
 * core/mem/raw.h にある (このヘッダはそれを include して store8/store16/store32 を使う)。
 *
 * host 対応: 原典は番地を (unsigned int)_addr と出力していたが、これは
 * 64bit host では「ポインタ→小さい整数」でコンパイルエラーになる。
 * to_addr32() で 2 段キャストし 32bit へ切り詰める (ARM 由来の 32bit 前提を保持)。
 *********************************************/
#include "core_config.h"   /* INLINE / size_ */
#include "conv.h"          /* printf */
#include "hal/io.h"        /* putchar */
#include "hal/mcheck.h"    /* mcheck_read / mcheck_write */
#include "raw.h"           /* store8 / store16 / store32 */

/* 番地を 32bit に切り詰めて出力用に (ARM 由来の 32bit アドレス前提) */
inline unsigned int to_addr32(const void* p){ return (unsigned int)(unsigned long)p; }

/* 可否検査つき型付きストア */
INLINE void checked_store(unsigned int _val, unsigned char* _addr, size_ _size){
	if (!mcheck_write(_addr)) return;
	switch (_size){
	case 1:
		printf("writing 8bit 0x%2x to _addr:%x\n", _val, to_addr32(_addr));
		store8(_val, _addr);
		break;
	case 2:
		printf("writing 16bit 0x%4x to _addr:%x\n", _val, to_addr32(_addr));
		store16(_val, (unsigned short*)_addr);
		break;
	case 4:
	default:
		printf("writing 32bit 0x%8x to _addr:%8x\n", _val, to_addr32(_addr));
		store32(_val, (unsigned int*)_addr);
		break;
	}
}

/* 型別ダンプ (dump_word の下請け, 検査なし) */
INLINE void dump8(unsigned char*  _addr){ printf("%x : %2x\n", to_addr32(_addr), *_addr); }
INLINE void dump16(unsigned short* _addr){ printf("%x : %4x\n", to_addr32(_addr), *_addr); }
INLINE void dump32(unsigned int*   _addr){ printf("%x : %x\n",  to_addr32(_addr), *_addr); }

/* 可否検査つき 1 語ダンプ */
INLINE void dump_word(void* _addr, size_ _size){
	if (!mcheck_read(_addr)) return;
	switch (_size){
	case 1:  dump8((unsigned char*)_addr);  break;
	case 2:  dump16((unsigned short*)_addr); break;
	case 4:
	default: dump32((unsigned int*)_addr);   break;
	}
}

/* 可否検査つき連続ダンプ (16 進, _sep_sz バイト区切り) */
#define MOUTN_SEP_SZ 0x10
INLINE void dump_range(void* addr, size_ _len, size_ _size = 4, unsigned int _sep_sz = MOUTN_SEP_SZ){
	if (_sep_sz == 0) _sep_sz = 0x10;
	if (!(_size == 1 || _size == 2)) _size = 4;
	for (unsigned int j = 0; j < _len; j += _sep_sz){
		if (!mcheck_read((unsigned char*)addr)) continue;
		printf("%x:", to_addr32((unsigned char*)addr + j));
		for (unsigned int k = 0; k < _sep_sz; k += _size){
			if (!mcheck_read((unsigned char*)addr + j + k)) continue;
			switch (_size){
			case 1: printf(" %2x ", *((unsigned char*)addr + j + k)); break;
			case 2: printf(" %4x", *((unsigned short*)((unsigned char*)addr + j + k))); break;
			case 4: printf(" %x",  *((unsigned int*)((unsigned char*)addr + j + k))); break;
			}
		}
		putchar('\n');
	}
}

#endif /* CORE_MEM_INSPECT_H */
