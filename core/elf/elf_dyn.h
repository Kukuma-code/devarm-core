#ifndef CORE_ELF_ELF_DYN_H
#define CORE_ELF_ELF_DYN_H
/*********************************************
 * core/elf: ELF dynamic entry (Elf64_Dyn) の型 + ダンプ宣言 (旧 devel include/elf_dyn.h)
 * d_un は val/ptr の union。
 *********************************************/
#include "elf_types.h"

class elf_dyn_{
 public:
	elf_s64_ tag;
	union {
		elf_u64_ val;
		elf_u64_ ptr;
	};
 public:
	void info();
};

#endif /* CORE_ELF_ELF_DYN_H */
