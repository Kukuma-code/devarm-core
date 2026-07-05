#ifndef CORE_ELF_ELF_EHDR_H
#define CORE_ELF_ELF_EHDR_H
/*********************************************
 * core/elf: ELF ヘッダ (Elf64_Ehdr レイアウト) の型 + ダンプ宣言
 *
 * 旧 devel include/elf_ehdr.h。原典は declclass.h の DECLCLASS(CLASSORDER スキーム)
 * によるクラス宣言 codegen だったが、elf クラスタは全て order=0 (非テンプレ) で
 * template class 対応を使わないため、スキームの概要だけ引き継いで実クラス定義に
 * 展開した (旧 include/declclass.h への依存を切り、core を自己完結にする)。
 * フィールドの並び・意味は ELF 仕様どおり (名前は原典の terse)。
 *********************************************/
#include "elf_types.h"   /* elf_u*_ / elf_addr_ / elf_ofs_ / EI_NIDENT */

class elf_ehdr_{
 public:
	union{
		unsigned char ident[EI_NIDENT];
		struct {
			char magic[4];
			char elf_class;
			char byte_order;
			char ident_version;
			char os_abi;
			char abi_version;
			char _pad[7];
		} __attribute__((__packed__));
	};
	elf_u16_ type;
	elf_u16_ machine;
	elf_u32_ version;
	elf_addr_ entry;
	elf_ofs_ phdr_ofs;
	elf_ofs_ shdr_ofs;
	elf_u32_ flags;
	elf_u16_ ehdr_size;
	elf_u16_ phdr_entsize;
	elf_u16_ phdr_qty;
	elf_u16_ shdr_entsize;
	elf_u16_ shdr_qty;
	elf_u16_ shdr_strndx;
	void info();
	void raw_data();
};

#endif /* CORE_ELF_ELF_EHDR_H */
