/*********************************************
 * core/elf: elf_phdr_ のダンプ実体。旧 devel lib/elf/elf_phdr.c を
 * <presettings.h> から新 core includes へ張替え。ロジック・token 表は原典と同一。
 * is_typename の戻り値は char* -> const char* (is_token_name の const 化に追随)。
 * 64bit フィールドの "%x" 表示は下位32bit のみ (elf_ehdr.cpp と同様の既知制限)。
 *********************************************/
#include "elf_types.h"
#include "aout.h"
#include "misc_token.h"
#include "elf_phdr.h"

namespace ELF{
	namespace PHDR{
#define TKP(x) TK(PT_##x)
		token_ tk_type[]={TKP(NULL),TKP(LOAD),TKP(DYNAMIC),TKP(INTERP),TKP(NOTE),
			TKP(SHLIB),TKP(PHDR),TKP(TLS),TKP(NUM),TKP(LOOS),
			TKP(GNU_EH_FRAME),TKP(GNU_STACK),TKP(GNU_RELRO),
			TKP(LOSUNW),TKP(SUNWBSS),TKP(SUNWSTACK),
			TKP(HISUNW),TKP(HIOS),
			TKP(LOPROC),TKP(HIPROC),TK_END()};
		token_ tk_flags[]={TK(PF_X),TK(PF_W),TK(PF_R)};
#undef TKP
	}
}
void elf_phdr_::print_flags(){
	elf_u32_ flags=this->flags;
	if (flags >=PF_R){printf("[read]");flags-=PF_R;}
	if (flags >=PF_W){printf("[write]");flags-=PF_W;}
	if (flags >=PF_X){printf("[exec]");flags-=PF_X;}
}
const char* elf_phdr_::is_typename(){return is_token_name(type,ELF::PHDR::tk_type);}
void elf_phdr_::info(){
#define TMP_PRINT(x) printf("\t%-16s:[%x:%s]\n",#x,x,is_token_name(x,ELF::PHDR::tk_##x))
	TMP_PRINT(type);
#define TMP_PRINT2(x) printf("\t%-16s:[%x]\n",#x,x)
	TMP_PRINT2(ofs);
	TMP_PRINT2(vaddr);
	TMP_PRINT2(paddr);
	TMP_PRINT2(file_size);
	TMP_PRINT2(mem_size);
	printf("\t%-16s:%x:","flags",flags);
	print_flags();
	printf("\n");
	TMP_PRINT2(align);
#undef TMP_PRINT
#undef TMP_PRINT2
}

