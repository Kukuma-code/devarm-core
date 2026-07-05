#ifndef CORE_ELF_ELF_SYM_H
#define CORE_ELF_ELF_SYM_H
/*********************************************
 * core/elf: ELF symbol (Elf64_Sym) の型 + ダンプ宣言 (旧 devel include/elf_sym.h)
 * _info は type:4 / bind:4 のビットフィールド union。packed で Elf64_Sym=24B に一致。
 *********************************************/
#include "elf_types.h"

class elf_sym_{
 public:
	elf_u32_ name;             /* Symbol name (string tbl index) */
	union{
		unsigned char _info;   /* Symbol type and binding */
		struct {
			unsigned char type:4;
			unsigned char bind:4;
		};
	} __attribute__((__packed__));
	unsigned char other;       /* Symbol visibility */
	elf_u16_ shdr_ndx;         /* Section index */
	elf_addr_ value;           /* Symbol value */
	elf_u64_  size;            /* Symbol size */
 public:
	void info();
} __attribute__((__packed__));

#endif /* CORE_ELF_ELF_SYM_H */
