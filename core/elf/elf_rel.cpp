/*********************************************
 * core/elf: elf_rel_ / elf_rela_ のダンプ実体。旧 devel lib/elf/elf_rel.c を
 * <presettings.h> から新 core includes へ張替え。ロジックは原典と同一
 * (TMP_PRINT は elf_rel_::info で定義し elf_rela_::info まで共用)。原典の THIS_FUNC は
 * 実メソッド定義へ展開済み (経緯は elf_ehdr.h 冒頭)。
 *********************************************/
#include "elf_types.h"
#include "aout.h"
#include "misc_token.h"
#include "elf_rel.h"

void elf_rel_::info(){
#define TMP_PRINT(x) printf("%-16s:[0x%x]\n",#x,x);
	TMP_PRINT(ofs);
	TMP_PRINT(type);
	TMP_PRINT(sym);
}

void elf_rela_::info(){
	TMP_PRINT(ofs);
	TMP_PRINT(type);
	TMP_PRINT(sym);
	TMP_PRINT(addend);
#undef TMP_PRINT
}

