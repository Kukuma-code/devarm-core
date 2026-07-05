#ifndef CORE_ELF_ELF_SHDR_H
#define CORE_ELF_ELF_SHDR_H
/*********************************************
 * core/elf: ELF section header (Elf64_Shdr) の型 + ダンプ宣言
 * 旧 devel include/elf_shdr.h。素の class 宣言 (原典のメソッド定義側 THIS_CTOR/
 * THIS_FUNC は実メソッド定義へ展開済み。経緯は elf_ehdr.h 冒頭)。
 *********************************************/
#include "elf_types.h"   /* elf_u*_ / elf_addr_ / elf_ofs_ */

class elf_shdr_{
 public:
	elf_u32_ name;
	elf_u32_ type;
	elf_u64_ flags;
	elf_addr_ addr;
	elf_ofs_ ofs;
	elf_u64_ size;
	elf_u32_ link;
	elf_u32_ _info;
	elf_u64_ addr_align;
	elf_u64_ entsize;
 public:
	elf_shdr_();
	const char* is_typename();
	void print_flags();
	void info();
};

#endif /* CORE_ELF_ELF_SHDR_H */
