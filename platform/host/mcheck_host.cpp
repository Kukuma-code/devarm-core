/*********************************************
 * platform/host: mcheck 継ぎ目のホスト実装 (フラットアドレス空間)
 *
 * ホストは連続した仮想アドレス空間を持ち、LPC2388 のような MMIO 番地レンジ
 * 判定は意味を持たない。呼び出し側を信頼して常に「可」を返す。
 * これにより core/mem/inspect.h の checked_store/dump_word/dump_range を host で実証できる。
 * LPC2388 実装は platform/lpc2388 が同じ継ぎ目を番地レンジで提供する (obsolete)。
 *********************************************/
#include "hal/mcheck.h"

static int host_can_read(const void*){ return 1; }
static int host_can_write(const void*){ return 1; }

/* 継ぎ目にホスト実装を差し込む */
extern "C" int (*mem_can_read)(const void*)  = host_can_read;
extern "C" int (*mem_can_write)(const void*) = host_can_write;
