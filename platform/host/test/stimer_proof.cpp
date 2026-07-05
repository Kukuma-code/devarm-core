/*********************************************
 * platform/host/test: core/util/stimer の回帰ハーネス (host)
 * base_tick を決定的なモックに差し替え、非ラップ/ラップ両方の
 * set_timer / istimer / iselapsed を厳密値で検証する。
 * ビルド例:
 *   clang++ -std=c++17 -Wall -Wextra -I core/include -I core/convert -I core/util -I core/io -I . \
 *     platform/host/test/stimer_proof.cpp core/util/stimer.cpp hal/timer.cpp \
 *     hal/io.cpp platform/host/timer_host.cpp platform/host/io_host.cpp -o stimer_proof
 *********************************************/
#include "stimer.h"
#include "hal/timer.h"
#include "aout.h"   /* puts / putchar / TRUE / FALSE (via core_config.h) */

static unsigned int g_tick = 0;
static unsigned int mock_tick(){ return g_tick; }

static int failed = 0;
static void check(const char* name, bool ok){
	puts(ok ? "[OK] " : "[FAIL] "); puts(name); putchar('\n');
	if (!ok) ++failed;
}

int main(){
	base_tick = mock_tick;   /* host クロックを決定的モックへ差し替え */

	stimer_ t;

	// --- 非ラップ: begin=100, 期間 50 -> cnt=150 ---
	g_tick = 100; t.set_timer(50);
	check("nonwrap begin=100", t.begin == 100u);
	check("nonwrap cnt=150",   t.cnt == 150u);
	g_tick = 120; check("nonwrap before deadline -> FALSE", t.istimer() == FALSE);
	g_tick = 120; check("nonwrap iselapsed remaining=30", t.iselapsed() == 30u);
	g_tick = 150; check("nonwrap at deadline -> FALSE", t.istimer() == FALSE);
	g_tick = 151; check("nonwrap past deadline -> TRUE", t.istimer() == TRUE);

	// --- ラップ: begin=0xFFFFFFF0, 期間 0x20 -> cnt=0x10 (2^32 跨ぎ) ---
	g_tick = 0xFFFFFFF0u; t.set_timer(0x20u);
	check("wrap begin=0xFFFFFFF0", t.begin == 0xFFFFFFF0u);
	check("wrap cnt=0x10 (wrapped)", t.cnt == 0x10u);
	g_tick = 0xFFFFFFF8u; check("wrap upper span -> FALSE", t.istimer() == FALSE);
	g_tick = 0x08u;       check("wrap lower span -> FALSE", t.istimer() == FALSE);
	g_tick = 0x20u;       check("wrap gap (past) -> TRUE", t.istimer() == TRUE);
	g_tick = 0xFFFFFFF8u; check("wrap iselapsed=0x18", t.iselapsed() == 0x18u);

	puts(failed ? "SOME FAILED\n" : "ALL PASS\n");
	return failed ? 1 : 0;
}
