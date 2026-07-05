# devarm-core

A portable, hardware-independent core salvaged from the author's own
LPC2388 (ARM7TDMI-S) firmware and its minimal libc, written in the early
2000s. The original ran bare-metal; this tree re-targets it as a generic
C++17 core whose only contact with the outside world is a small
function-pointer HAL, so the same code builds and is regression-tested on
a plain host (macOS/Linux) today and can be re-seated on an MCU later.

Source comments are written in Japanese. They document, for each module,
the original design intent and every deviation this port makes from the
original (modernizations, bug fixes, ABI/alignment adaptations).

Sibling project: [uni-toolkit](https://github.com/Kukuma-code/uni-toolkit)
rebuilds the template/container layer of the same original codebase as
native C++17; devarm-core is the minimal bare-metal-oriented slice.

## Layout

- `core/` — the portable core (no hardware, no OS assumptions)
  - `string/`, `block/` — ASCII string and raw byte-block primitives
  - `convert/` — number/string conversions (dec/hex/bin, 32/64-bit, double)
  - `io/` — `printf` family built on a single `putchar` seam
  - `mem/` — region allocator (`mallocator_`) with adjacent-free-node
    coalescing, global `operator new`/`delete` and `malloc`/`realloc`
    wiring, checked memory inspection/dump helpers
  - `cond/` — character/string predicates
  - `elf/` — ELF64 structure readers/dumpers (fixed-width layout, spec
    constants vendored; no system `<elf.h>` dependency)
  - `util/` — token-name resolution helper
- `hal/` — the hardware seam: function-pointer contracts for byte I/O
  (`base_out`/`base_in`), heap bounds (`heap_begin`/`heap_end`), memory
  probing (`mem_can_read`/`write`) and a free-running 32-bit tick
  (`base_tick`)
- `platform/host/` — host implementations of the seams (stdout/stdin,
  static heap arena, monotonic clock) and the regression proofs

## Testing

```
python3 platform/host/test/run_proofs.py
```

Self-contained runner, no dependencies beyond `clang++` and Python 3.
It builds every proof with `-std=c++17 -Wall -Wextra` (warnings are
failures), runs it, and aggregates results. Proofs assert their own
ground truth, including a real embedded ELF64 relocatable for the ELF
readers — no execution environment for ELF is required, since the ELF
layer's contract is byte interpretation only.

## License

MIT — see [LICENSE](LICENSE).
