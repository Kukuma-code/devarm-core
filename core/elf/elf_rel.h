#ifndef CORE_ELF_ELF_REL_H
#define CORE_ELF_ELF_REL_H
/*********************************************
 * core/elf: ELF relocation (Elf64_Rel / Elf64_Rela) の型 + ダンプ宣言
 * 旧 devel include/elf_rel.h。_info は {type, sym} の union (LE で (sym<<32)|type に一致)。
 *********************************************/
#include "elf_types.h"

class elf_rel_{
 public:
	elf_addr_ ofs;             /* Address */
	union {
		elf_u64_ _info;        /* Relocation type and symbol index */
		struct {
			elf_u32_ type;
			elf_u32_ sym;
		};
	};
 public:
	void info();
};

class elf_rela_{
 public:
	elf_addr_ ofs;             /* Address */
	union {
		elf_u64_ _info;        /* Relocation type and symbol index */
		struct {
			elf_u32_ type;
			elf_u32_ sym;
		};
	};
	elf_s64_ addend;           /* Addend */
 public:
	void info();
};

#endif /* CORE_ELF_ELF_REL_H */
