/*********************************************
 * core/elf: elf_ehdr_ のダンプ実体 (info / raw_data)
 *
 * 旧 devel lib/elf/elf_ehdr.c を <presettings.h> (旧モノリシック libc チェーン)
 * 依存から、新 core の必要ヘッダへ張替えたもの:
 *   elf_types.h(幅型・定数) /
 *   aout.h(printf・puts・puth と、それが引く core_config の size_/qty_・hal/io の putchar) /
 *   misc_token.h(token_/TK/is_token_name)。
 * ロジック・token テーブルは原典と同一。原典の THIS_FUNC(declclass.h の codegen) は
 * 実メソッド定義 (elf_ehdr_::) へ展開済み (経緯は elf_ehdr.h 冒頭)。
 *
 * ELF64 表示の注意: entry/phdr_ofs/shdr_ofs は 64bit (elf_addr_/elf_ofs_) だが、
 * 原典どおり "%x" で出力するため下位 32bit のみ表示になる (custom printf は format 検査外)。
 * 64bit 完全表示は実 ELF64 での値検証段で対応する。
 *********************************************/
#include "elf_types.h"
#include "aout.h"
#include "misc_token.h"
#include "elf_ehdr.h"

namespace ELF{
	namespace EHDR{
		token_ tk_elf_class[] ={TK(ELFCLASSNONE),TK(ELFCLASS32),TK(ELFCLASS64),TK_END()};
		token_ tk_byte_order[]={TK(ELFDATANONE),TK(ELFDATA2LSB),TK(ELFDATA2MSB),TK_END()};
		token_ tk_version[]={TK(EV_NONE),TK(EV_CURRENT),TK_END()};
		token_ tk_type[]={TK(ET_NONE),TK(ET_REL),TK(ET_EXEC),TK(ET_DYN),TK(ET_CORE),TK_END()};
		token_ tk_machine[]={TK(EM_NONE),TK(EM_M32),TK(EM_SPARC),TK(EM_386),TK(EM_68K),TK(EM_88K),
			TK(EM_860),TK(EM_MIPS),
			TK(EM_PARISC),TK(EM_SPARC32PLUS),TK(EM_PPC),TK(EM_ALPHA),TK(EM_SPARCV9),
			TK(EM_VAX),TK(EM_X86_64),TK_END()};
	}
}

void elf_ehdr_::info(){
	printf("IDENT(\n");
	printf("\tMAGIC:");
	for (size_ i = 0; i <= EI_MAG3; i++) {printf(" ['%c' %x]", ident[i], ident[i]);}
	printf("\n");
#define TMP_PRINT(x) printf("\t%-16s:[0x%x:%s]\n",#x,x,is_token_name(x,ELF::EHDR::tk_##x))
#define TMP_PRINT2(x) printf("\t%-16s:[0x%x]\n",#x,x);
	TMP_PRINT(elf_class);
	TMP_PRINT(byte_order);
	printf("\t%-16s:[0x%x:%s]\n","elf_version",ident_version,is_token_name(ident_version,ELF::EHDR::tk_version));
	TMP_PRINT2(os_abi);
	TMP_PRINT2(abi_version);
	printf(")\n");
	TMP_PRINT(type);
	TMP_PRINT(machine);
	TMP_PRINT(version);
	TMP_PRINT2(entry);
	TMP_PRINT2(phdr_ofs);
	TMP_PRINT2(shdr_ofs);
	TMP_PRINT2(flags);
	TMP_PRINT2(ehdr_size);
	TMP_PRINT2(phdr_entsize);
	TMP_PRINT2(phdr_qty);
	TMP_PRINT2(shdr_entsize);
	TMP_PRINT2(shdr_qty);
	TMP_PRINT2(shdr_strndx);
#undef TMP_PRINT
#undef TMP_PRINT2
}

void elf_ehdr_::raw_data(){
	qty_ qty=sizeof(elf_ehdr_);
	for (qty_ i=0;i<qty;++i){
		putchar(i%16? ' ':'\n');
		puth(((char*)this)[i],2);
	}
	putchar('\n');
}
