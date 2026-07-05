/*********************************************
 * core/mem: グローバル operator new/delete を region allocator (mallocator) へ配線
 *
 * 旧 devel lib/mem/{new,delete,new_a,delete_a}.c を host / C++17 向けに統合移植。
 * 原典からの変更:
 *   1) 動的例外指定を除去 (C++17 で削除): new の throw(std::bad_alloc) と
 *      delete の throw() を落とす。new は std::bad_alloc を送出しうる (暗黙 noexcept(false))、
 *      delete は noexcept。
 *   2) 引数を標準シグネチャの std::size_t に統一 (原典は LONG64/ARM_ELF で分岐)。
 *      mallocator.alloc は size_(unsigned int) を取るため size_t から縮みうるが、
 *      通常サイズでは問題なし。
 *   3) C++14 の sized delete も提供 (size は使わず dealloc へ)。コンパイラが sized 版を
 *      呼ぶことがあるため。
 *   4) new[] の CLASS_ARRAY フラグ (原典 mnode_set_newcaflag) は devarm 最小 mnode が
 *      refcnt/CLASS_ARRAY を持たないため未対応。new[] は new と同じく alloc へ配線する。
 *   5) malloc/free/realloc エイリアス (原典 FUNC_ALIAS inline asm 依存) は host では未移植。
 *
 * 注意: mallocator.alloc(0) は std::bad_alloc を送出する (原典の alloc と同じ)。標準の
 *   new(0) 要件 (非 null 返却) とは異なるが、実利用 (welf 等) は size>0 のため顕在化しない。
 *   delete(nullptr) は dealloc の下限判定 (heap_begin>=_ref で return) により安全な no-op。
 *********************************************/
#include <new>
#include <cstddef>
#include "mallocator.h"

void* operator new(std::size_t size)   { return (void*)mallocator.alloc((size_)size); }
void* operator new[](std::size_t size) { return (void*)mallocator.alloc((size_)size); }

void operator delete(void* ref) noexcept   { mallocator.dealloc((unsigned char*)ref); }
void operator delete[](void* ref) noexcept { mallocator.dealloc((unsigned char*)ref); }

/* C++14 sized delete (size は使わない) */
void operator delete(void* ref, std::size_t) noexcept   { mallocator.dealloc((unsigned char*)ref); }
void operator delete[](void* ref, std::size_t) noexcept { mallocator.dealloc((unsigned char*)ref); }
