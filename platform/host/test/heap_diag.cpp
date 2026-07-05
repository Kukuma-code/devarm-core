/*********************************************
 * platform/host/test: region allocator の内部状態を実測する診断 (host, 非回帰)
 * 再利用時に current(bump 先端) がどう動き、[旧current, calc) が
 * 後続確保で使われるか (= 死に領域か否か) を配置から判定する。
 *********************************************/
#include "mallocator.h"

static unsigned long off(const void* p){ return (unsigned long)((const unsigned char*)p - heap_begin); }
static MNODE* node_of(unsigned char* userptr){ return (MNODE*)(userptr - sizeof(MNODE)); }

int main(){
	printf("sizeof(MNODE)=%d, heap_begin=%p, heap_end=%p, span=%d\n",
	       (int)sizeof(MNODE), (void*)heap_begin, (void*)heap_end,
	       (int)(heap_end - heap_begin));

	mallocator.init();
	const unsigned int S = 200;

	unsigned char* p0 = mallocator.alloc(S);
	unsigned char* p1 = mallocator.alloc(S);
	unsigned char* p2 = mallocator.alloc(S);
	unsigned char* p3 = mallocator.alloc(S);
	printf("after 4 allocs (S=%u, require=%u each):\n", S, (unsigned)(S + sizeof(MNODE)));
	printf("  node offsets: p0=%lu p1=%lu p2=%lu p3=%lu\n",
	       off(node_of(p0)), off(node_of(p1)), off(node_of(p2)), off(node_of(p3)));
	mallocator.info();

	mallocator.dealloc(p0);
	mallocator.dealloc(p1);
	mallocator.dealloc(p2);
	printf("after freeing p0,p1,p2:\n");
	mallocator.info();

	// 再利用を狙った確保
	unsigned char* r = mallocator.alloc(S);
	printf("reuse alloc -> r at node offset %lu (p0 node was %lu) : %s\n",
	       off(node_of(r)), off(node_of(p0)),
	       node_of(r) == node_of(p0) ? "REUSED p0 slot" : "did NOT reuse p0");
	mallocator.info();

	// 次の非再利用確保が current(=calc) に置かれるなら [旧current, calc) は死に領域
	unsigned char* q = mallocator.alloc(S);
	printf("next alloc -> q at node offset %lu\n", off(node_of(q)));
	printf("  (p3 node ended at offset %lu; if q >> that, frontier jumped past a gap)\n",
	       off(node_of(p3)) + S + sizeof(MNODE));
	mallocator.info();

	return 0;
}
