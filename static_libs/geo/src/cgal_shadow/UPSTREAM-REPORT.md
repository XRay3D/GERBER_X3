# `std::call_once` in `Lazy_rep::exact()` serializes all exact-kernel work on futex-less platforms (mingw-w64)

Draft issue/PR text for upstream CGAL (github.com/CGAL/cgal). The
accompanying patch is `cgal-lazy-call-once.patch` (against v6.2,
`Filtered_kernel/include/CGAL/Lazy.h`). Русское резюме — в шапке нашей
теневой копии `CGAL/Lazy.h` рядом с этим файлом.

## Summary

`Lazy_rep::exact()` guards lazy exactification with `std::call_once`
(Lazy.h, three sites). In libstdc++, `std::call_once` has a fast futex
path **on Linux only**; every other platform — notably Windows with
mingw-w64 — uses the generic fallback, which for *every completing
`once_flag`* acquires one **process-global mutex** and broadcasts one
**process-global condition variable** (a kernel wake syscall via
winpthreads `SetEvent`).

A lazy DAG node's flag completes exactly once, but an exact-kernel
workload (EPECK, `General_polygon_set_2` over circle-segment traits)
creates **millions of nodes**, so each of them pays a syscall, and —
much worse — every thread doing exactification serializes on that single
process-wide lock.

## Observed impact (Windows 11, mingw-w64 UCRT64, GCC 16.2, CGAL 6.2, 32 HW threads)

PCB pocketing workload (repeated `join`/`difference`/offset over arc
polygons), ETW trace with symbolized stacks:

* single-threaded run: ~1/3 of all CPU samples are in stacks ending in
  `SetEvent`, called from `Lazy_exact_*::update_exact` /
  `Real_embeddable_traits::Sgn` / `Sqrt_extension::compare`;
* thread scaling is **negative at any width**: 1 thread 2.29 s,
  2 threads — 2× slower, 32 threads — 5.7× slower than one; a 1.5 GB
  context-switch storm in the 8-thread trace (threads fighting for the
  global `once` mutex);
* on a real board the same computation took 187 s versus ~14 s on a
  comparable Linux box — entirely attributable to this effect.

With `std::call_once` replaced by a per-flag atomic once (patch below):

* single-threaded time halves (4.7 s → 2.4 s on our benchmark);
* parallel scaling becomes positive again (8 threads: 1.21 s);
* end-to-end: the 187 s board computation dropped to 8.0 s;
* results are bit-identical (the guard's semantics are unchanged).

## Fix

Replace the `std::once_flag` member with
`std::atomic<unsigned char>` (states: 0 not run / 1 running / 2 done /
3 running-with-waiters) and a small `internal::lazy_call_once`:

* fast path — one acquire load (same as the futex fast path on Linux);
* the winner runs `update_exact()` and wakes waiters **only if a waiter
  actually announced itself**, so the uncontended case never enters the
  kernel;
* an exception resets the flag to 0, matching `std::call_once`;
* contended waiting uses C++20 `atomic::wait/notify` when available
  (`__cpp_lib_atomic_wait`), with a yield loop as the C++17 fallback.

A related note already exists in Lazy.h ("The test is unnecessary, only
use it if benchmark says so" above `exact()`, and the long comment in
`Lazy_rep` discussing a CAS-based alternative) — this patch is
essentially that alternative, motivated by measurements.

The patch also shrinks `Lazy_rep` by `sizeof(std::once_flag) - 1` bytes
per node, which is a small win on all platforms including Linux.

## Reproducing

Any EPECK-heavy Boolean-set workload on mingw-w64 shows it; the quickest
check is a profiler: stacks of a single-threaded run are full of
`SetEvent` under `update_exact`. On Linux the futex path hides the
problem (a syscall only on actual contention), though the global
condvar broadcast still exists in the generic path for all other
futex-less targets.
