#ifndef CORE_MEM_MALLOCATOR_H
#define CORE_MEM_MALLOCATOR_H
/*********************************************
 * core/mem: region allocator (移植可能・host ビルド可)
 *
 * 旧 include/mallocator.h の bump + 再利用アロケータを host 移植したもの。
 * ヒープ領域 [heap_begin, heap_end) 上に MNODE ヘッダ付きのブロックを並べ、
 * dealloc 時に隣接する空きノードを併合する。アルゴリズムは原典と同一。
 *
 * 原典からの変更 (host 対応・移植のため):
 *   1) 領域境界を継ぎ目化: &allocbegin/&allocend (リンカラベル番地) ->
 *      hal/heap.h の heap_begin/heap_end (実行時ポインタ)。
 *   2) ポインタ演算を uintptr_t 化: 原典は (unsigned int) でポインタを
 *      切って計算しており 64bit host で破綻する。uintptr_t へ統一 (32bit
 *      ARM では uintptr_t==unsigned int なので挙動不変)。
 *   3) 例外を標準化: throw std::bad_alloc(ERR_C(code)) は std:: を独自
 *      再定義する exception.h に依存し host の libc++ と衝突する。標準
 *      <new> の std::bad_alloc() へ置換 (原典のエラー種別コードは失う)。
 *   4) DBG_MALLOCATOR_* のデバッグ計装を除去 (元々 no-op)。
 *
 * 補強 (原典の判定漏れを修正, bug_log id=95):
 *   原典の alloc() は空きノード再利用時も current (frontier) を calc まで
 *   前進させ、その結果 enlarge で予約 (current_bound) まで無駄に広げていた。
 *   share ベースの再利用判定 (has_space の HAS_SPACE) は「再利用モードに
 *   入るか」だけを見ており、「再利用成立時は frontier/予約を動かさない」
 *   という帰結の分岐が欠けていた。本実装では再利用パスと bump パスを分離し、
 *   再利用時は share のみ加算する。これにより:
 *     - 再利用が予約を無駄に拡張しない (低メモリ機で予約が最大へ張り付くのを防ぐ)
 *     - [begin, current) が常にヘッダ連続 (再利用時に穴を作らない) となり、
 *       非ゼロ初期化 RAM 上でも next_reuse_node の走査が破綻しない
 *   share += require ("現在使用量") は再利用でも増えるのが正しく、従来どおり。
 *   なお副次的挙動変更: 再利用で満たせる要求は、frontier が heap_end 間際でも
 *   OOM とせず先に再利用を試みる (低メモリ機では望ましい)。
 *
 * 整列保証 (アライメント対応, HANDOFF TODO B):
 *   原典は require = _size + sizeof(MNODE) を境界丸めせず byte-packed で並べて
 *   いた。奇数 _size の直後のノード (と返却 user ポインタ) が未整列になり、
 *   double / long long / Cortex-M の LDRD/STRD (8 境界必須, フォルト回避不可) や
 *   ARM7 の非境界未定義動作で破綻しうる。本実装では:
 *     1) alloc() で _size を MALLOCATOR_ALIGN (既定 8) へ切り上げる。
 *        sizeof(MNODE)=8 も MALLOCATOR_ALIGN の倍数 (下記 static_assert) なので、
 *        整列した heap_begin から始めれば全ノード・全 user ポインタが整列を保つ。
 *     2) heap_begin 自体の整列は platform 側の責務 (host: heap_host.cpp の
 *        alignas、契約は hal/heap.h に明記)。init()/dealloc の
 *        "current=heap_begin" 経路も整列前提で一貫する。
 *   既定を alignof(max_align_t) (host では 16) ではなく 8 とするのは、対象が
 *   32bit 組み込みで 8 が double/long long/LDRD-STRD/ポインタを満たす最小境界で
 *   あり、16 にするとヘッダも 16 へパディングが要り alloc 毎に 8 バイト無駄になる
 *   ため。16 境界が要る型を host proof で扱う場合のみ MALLOCATOR_ALIGN を上書きする。
 *********************************************/
#include <new>            /* std::bad_alloc */
#include "core_config.h"  /* INLINE / size_ */
#include "conv.h"         /* printf (info 用) */
#include "hal/heap.h"     /* heap_begin / heap_end */
#include "mnode.h"        /* MNODE / MALLOCATOR / NEXT_MNODE */

#define PAGE_SZ 1024
#ifndef MALLOCATOR_ALLOC_LEAST
#define MALLOCATOR_ALLOC_LEAST 1
#endif
#define TMP_MNODE_LEAST_SZ 16

/* 確保の整列境界 (既定 8)。malloc 相当の「任意の型に適した整列」を 32bit
   組み込み向けに保証する最小値。理由の詳細はファイル冒頭コメント参照。 */
#ifndef MALLOCATOR_ALIGN
#define MALLOCATOR_ALIGN 8u
#endif

/* ヘッダ直後の user ポインタ (node + sizeof(MNODE)) を整列させるには、
   ヘッダ自体が境界の倍数である必要がある。 */
static_assert(sizeof(MNODE) % MALLOCATOR_ALIGN == 0,
              "sizeof(MNODE) must be a multiple of MALLOCATOR_ALIGN");

/* v を a (2 の冪) の倍数へ切り上げる */
static inline unsigned int malloc_align_up(unsigned int v){
	return (v + (MALLOCATOR_ALIGN - 1u)) & ~(MALLOCATOR_ALIGN - 1u);
}

class mallocator_ {
private:
	unsigned char* current;         /* bump 先端 (未使用領域の先頭) */
	unsigned char* current_bound;   /* 現在確保済みの上限 */
	MNODE* reusep;                  /* 再利用探索の開始ノード */
	int state;
	unsigned int size;              /* enlarge で伸ばした総容量 */
	unsigned int share;             /* 使用中バイト数 (ヘッダ込み) */

public:
	/* 遅延初期化対応: ELF (Linux) では core/mem/malloc.cpp の malloc が process
	   全体へ interpose され、共有ライブラリの初期化 (libstdc++ の EH pool 等) から
	   本コンストラクタより先に呼ばれうる。その場合は try_alloc 入口の ensure_init()
	   で初期化済みなので、ここで init() し直すと先行確保を孤児化する。静的記憶域は
	   動的初期化前にゼロ初期化されるため current==0 が「未初期化」を表す。
	   入口は malloc ではなく try_alloc に置く: operator new (alloc) / realloc も
	   同じ遅延初期化を通る。 */
	mallocator_(){ ensure_init(); }
	~mallocator_(){}

	bool ready() const { return current != 0; }
	void ensure_init(){ if ( !ready() ) init(); }

	void init(){
		state = MALLOCATOR::CLEAN; size = 0; share = 0;
		current = current_bound = heap_begin;
		reusep = (MNODE*)current_bound;
	}

	void info(){
		printf("[mallocator] current:%p bound:%p reusep:%p size:%d share:%d state:%x\n",
		       (void*)current, (void*)current_bound, (void*)reusep, size, share, state);
	}

	int has_space(){
		unsigned int diff = (unsigned int)(uintptr_t)(current - heap_begin);
		if ( share < (diff / 2) && diff > (size / 2) ){
			state |= MALLOCATOR::HAS_SPACE;
		} else if ( share >= diff ){
			state &= ~MALLOCATOR::HAS_SPACE;
			reusep = (MNODE*)current_bound;
		}
		return (state & MALLOCATOR::HAS_SPACE);
	}

	MNODE* next_reuse_node(size_ _size){
		MNODE* tmp = reusep;
		while ( (unsigned char*)tmp < current ){
			if ( (tmp->state & MALLOCATOR::USED) == 0 && tmp->size >= _size ){
				if ( tmp->size > _size + sizeof(MNODE) + TMP_MNODE_LEAST_SZ ){
					/* 分割: 残余を新ノード化し、再利用ノードを _size へ切り詰める。
					   size の切詰めは「残余ノードを実際に作る」この分割時のみ行う。
					   非分割時に切り詰めると穴の端数バイトがどのノードにも属さず孤児化し、
					   node 末尾 != 次ノード header となって走査が破綻する (bug_log id=99)。 */
					reusep = (MNODE*)((uintptr_t)tmp + _size + sizeof(MNODE));
					reusep->state &= ~MALLOCATOR::USED;
					reusep->size = tmp->size - _size - sizeof(MNODE);
					tmp->size = _size;
				}
				return tmp;
			}
			NEXT_MNODE(tmp);
		}
		return (MNODE*)0;
	}

	/* 予約を広げる。失敗時は 0 を返す (送出しない: try_alloc 参照)。 */
	size_ enlarge(){
		if ( current_bound == heap_end ) return 0;
		unsigned int alloc_size = size + MALLOCATOR_ALLOC_LEAST * PAGE_SZ;
		uintptr_t addr = (uintptr_t)current_bound + alloc_size;
		if ( addr < (uintptr_t)heap_begin ) return 0;
		if ( (uintptr_t)heap_end < addr ){
			size += (unsigned int)(uintptr_t)(heap_end - current_bound);
			current_bound = heap_end;
		} else {
			current_bound = (unsigned char*)addr;
			size += alloc_size;
		}
		return alloc_size;
	}

	/* operator new 用: 失敗時に std::bad_alloc を送出する (原典と同じ意味論)。 */
	unsigned char* alloc(size_ _size){
		unsigned char* p = try_alloc(_size);
		if ( p == 0 ) throw std::bad_alloc();
		return p;
	}

	/* malloc 用: 失敗時に 0 を返し、送出しない。
	   C++ 実行時 (libstdc++ の __cxa_allocate_exception や emergency pool) は
	   malloc を使うため、本 allocator が process の malloc を担う環境 (bare-metal、
	   ELF のシンボル interpose) で malloc 内から送出すると、例外オブジェクトの確保が
	   malloc へ再入して OOM -> throw -> malloc ... と無限再帰する。また Linux では
	   mallocator の動的初期化前に libstdc++ の初期化が malloc を呼ぶ (未初期化状態は
	   current==current_bound==0)。入口の ensure_init() でその場で初期化して応える
	   (旧版はここで 0 を返していた。送出しないことは変わらない)。 */
	unsigned char* try_alloc(size_ _size){
		if ( _size == 0 ) return 0;
		ensure_init();   /* 動的初期化前の呼出し (ELF interpose) に備えた遅延初期化 */
		/* 要求サイズを境界へ切り上げ。以降 _size/require/tmp->size は全て
		   MALLOCATOR_ALIGN の倍数となり、整列した heap_begin から並べる限り
		   全ノード・全 user ポインタが整列を保つ (再利用時の分割・併合も倍数
		   同士の加減算なので整列が崩れない)。 */
		unsigned int aligned = malloc_align_up(_size);
		if ( aligned < _size ) return 0;   /* 丸めのオーバフロー */
		_size = aligned;
		unsigned int require = _size + sizeof(MNODE);

		/* 再利用パス (share ベースの HAS_SPACE 判定で選択): 予約済み領域
		   [begin, current) 内の空きノードを再利用する。この場合 frontier
		   (current) も予約 (current_bound) も動かさず、使用量 share だけ
		   増やす。空きノードは既に予約済みなので追加予約・境界チェックは不要。 */
		if ( state & MALLOCATOR::HAS_SPACE ){
			MNODE* hole = next_reuse_node(_size);
			if ( hole ){
				hole->state = MALLOCATOR::USED + 1;   /* USED + refcnt(1) */
				/* hole->size は next_reuse_node が確定させている: 分割時は _size に
				   切詰め済み、非分割時は穴の全スパンを保持する。ここで _size を無条件
				   に上書きすると非分割時に端数を孤児化する (bug_log id=99)。
				   share は実ノード占有量 (payload + header) で会計する。dealloc は
				   同じ hole->size で減算するため、これで加減算が一致する。 */
				share += hole->size + sizeof(MNODE);
				return (unsigned char*)((uintptr_t)hole + sizeof(MNODE));
			}
		}

		/* bump パス: frontier を前進させる。予約が足りなければ enlarge で
		   後方 (heap_end 方向) へ広げる。 */
		uintptr_t calc = (uintptr_t)current + require;
		if ( (unsigned char*)calc >= heap_end ) return 0;
		while ( calc > (uintptr_t)current_bound ) if ( enlarge() == 0 ) return 0;
		MNODE* tmp = (MNODE*)current;
		tmp->state = MALLOCATOR::USED + 1;   /* USED + refcnt(1) */
		tmp->size = _size;
		current = (unsigned char*)calc;
		share += require;
		return (unsigned char*)((uintptr_t)tmp + sizeof(MNODE));
	}

	void dealloc(unsigned char* _ref){
		if ( heap_begin >= _ref || current_bound <= _ref ) return;
		MNODE* tmp = (MNODE*)((uintptr_t)_ref - sizeof(MNODE));
		/* 冪等ガード: 既に解放済みのノードを二重 dealloc しても no-op とする。
		   欠くと double-free で share が unsigned アンダーフローし、share==0 の
		   全解放リセット経路が死に、併合も二重加算されて走査が破綻する (bug_log id=100)。 */
		if ( !(tmp->state & MALLOCATOR::USED) ) return;
		tmp->state &= 0;
		share -= tmp->size + sizeof(MNODE);
		/* 後続の空きノードを併合 */
		MNODE* tmp2 = (MNODE*)((uintptr_t)tmp + tmp->size + sizeof(MNODE));
		while ( (unsigned char*)tmp2 < current && !(tmp2->state & MALLOCATOR::USED) ){
			tmp->size += tmp2->size + sizeof(MNODE);
			tmp2 = (MNODE*)((uintptr_t)tmp + sizeof(MNODE) + tmp->size);
		}
		if ( share == 0 ){
			current = heap_begin;
			reusep = (MNODE*)current_bound;
		} else if ( tmp < reusep ){
			reusep = tmp;
		}
		has_space();
	}
};

extern mallocator_ mallocator;   /* 単一ヒープのシングルトン (定義は mallocator.cpp) */

#endif /* CORE_MEM_MALLOCATOR_H */
