#ifndef CORE_ELF_ELF_PHDR_H
#define CORE_ELF_ELF_PHDR_H
/*********************************************
 * core/elf: ELF program header (Elf64_Phdr) の型 + ダンプ宣言
 * 旧 devel include/elf_phdr.h。原典は declclass.h の DECLCLASS(CLASSORDER スキーム)
 * だったが order=0 (非テンプレ) のみのため、概要だけ引き継いで実クラス定義に展開
 * (旧 include/declclass.h への依存を切る。経緯の詳細は elf_ehdr.h 冒頭)。
 * (原典末尾の PHDR_* コメントアウト定数は削除。値は elf_types.h の PT_* にある)
 *********************************************/
#include "elf_types.h"   /* elf_u*_ / elf_addr_ / elf_ofs_ */

class elf_phdr_{
 public:
	elf_u32_ type;
	elf_u32_ flags;
	elf_ofs_ ofs;
	elf_addr_ vaddr;
	elf_addr_ paddr;
	elf_u64_ file_size;
	elf_u64_ mem_size;
	elf_u64_ align;
 public:
	const char* is_typename();
	void print_flags();
	void info();
};

#endif /* CORE_ELF_ELF_PHDR_H */
