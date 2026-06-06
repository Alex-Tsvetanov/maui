# The Complete C/C++ Sanitizers Handbook

[![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png)](https://creativecommons.org/publicdomain/zero/1.0/)

*Disclaimer: ChatGPT generated document*.

Below is the guide I would want if I were onboarding a senior C++ engineer to sanitizers and wanted both practical depth and architectural understanding.

## 1. What sanitizers are

A sanitizer is a compiler-plus-runtime instrumentation system that modifies your program so it can detect classes of bugs while the program runs. In practice, the compiler inserts checks around operations that are otherwise unchecked in C and C++, and a runtime library manages metadata, intercepts allocations and library calls, and reports failures with stack traces and object history. Clang documents this model explicitly for ASan, TSan, MSan, UBSan, and others; GCC exposes the same family of instrumentation through `-fsanitize=...`; MSVC currently documents production support around AddressSanitizer. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

This makes sanitizers fundamentally different from static analysis. Static analysis reasons about code without running it. Sanitizers observe actual executions. That means they miss paths you do not execute, but they catch real dynamic failures with concrete values, stacks, allocations, and thread interleavings. For C++ work, that tradeoff is usually excellent: you run tests, fuzzers, integration suites, or targeted repros under instrumentation and fix what the runtime proves is wrong.

The most important mental model is this: sanitizers are not “a debug mode.” They are specialized dynamic analyses that instrument specific categories of behavior. You choose them based on the bug class you are hunting.

------

## 2. Why sanitizers matter so much in C and C++

C and C++ let you write extremely fast and extremely unsafe code. That is not a complaint; it is the price of control. Raw pointers, unchecked arithmetic, object lifetime ambiguity, aliasing rules, data races, uninitialized storage, and UB-driven optimization all create failure modes that often:

- do not crash where the bug occurs,
- disappear under a debugger,
- survive unit tests,
- only appear under optimization,
- look nondeterministic,
- or become exploitable security bugs.

Sanitizers drastically reduce the time from “we have a weird crash in prod” to “here is the exact buggy access, allocation site, free site, and stack.”

For a C++ engineer, the practical value is enormous in memory management, containers, iterators, ownership transfers, polymorphism, concurrency, low-level serialization, custom allocators, networking, and ABI boundaries.

------

## 3. The big sanitizer families

The core families today are:

- **AddressSanitizer (ASan)**: memory safety bugs such as out-of-bounds, use-after-free, use-after-return in many cases, and related heap/stack/global memory issues. Clang and GCC support it; MSVC supports ASan as its main sanitizer offering. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))
- **LeakSanitizer (LSan)**: memory leak detection, often combined with ASan. Clang documents both combined and standalone modes. GCC also documents `-fsanitize=leak`. ([Clang](https://clang.llvm.org/docs/LeakSanitizer.html))
- **UndefinedBehaviorSanitizer (UBSan)**: dynamic checks for many forms of UB or suspicious behavior, such as null, alignment, bounds, certain shift issues, signed overflow, and more. It supports recovery and trap modes. ([Clang](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html))
- **ThreadSanitizer (TSan)**: data race detection for multithreaded code, with significant runtime and memory overhead. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))
- **MemorySanitizer (MSan)**: use of uninitialized memory, especially poison propagation into branches, pointer dereferences, parameters, and library boundaries. ([Clang](https://clang.llvm.org/docs/MemorySanitizer.html))
- **HWAddressSanitizer (HWASan)**: a hardware-assisted memory safety variant available on supported targets such as AArch64, using tagged pointers/top-byte-ignore style capabilities. GCC documents it as AArch64-only and incompatible with `thread` and `address`. ([Clang](https://clang.llvm.org/docs/HardwareAssistedAddressSanitizerDesign.html?utm_source=chatgpt.com))
- **TypeSanitizer**: catches strict-aliasing violations using type-based alias analysis metadata. Clang documents it as aimed at finding code that only “works” because optimizations have not yet exploited aliasing rules too aggressively. ([Clang](https://clang.llvm.org/docs/TypeSanitizer.html))
- **DataFlowSanitizer (DFSan)**: taint/data-flow tracking framework rather than a bug detector by itself. ([Clang](https://clang.llvm.org/docs/DataFlowSanitizer.html?utm_source=chatgpt.com))
- **RealtimeSanitizer (RTSan)**: checks real-time safety violations in functions marked `[[clang::nonblocking]]`; Clang recommends pairing it with compile-time Function Effect Analysis. ([Clang](https://clang.llvm.org/docs/RealtimeSanitizer.html))
- **SanitizerCoverage**: not a bug detector itself, but low-overhead instrumentation frequently used with fuzzing and sanitizers to expose coverage information at function/block/edge level. ([Clang](https://clang.llvm.org/docs/SanitizerCoverage.html))

There are also related instrumentation/security features that people loosely group nearby, but they are not “the sanitizer suite” in the same sense: SafeStack, shadow call stack, stack protectors, Control Flow Integrity, source-based coverage, static thread-safety analysis, etc. They are adjacent, not equivalent. Clang’s docs separate them accordingly. ([Clang](https://clang.llvm.org/docs/SafeStack.html?utm_source=chatgpt.com))

------

## 4. The single most important rule: sanitizers are per-bug-class tools

A lot of teams ask, “Which sanitizer should always be on?” The right answer is not one sanitizer. It is a **matrix**:

- **ASan + UBSan** for most day-to-day C++ testing.
- **TSan** for dedicated concurrency jobs.
- **MSan** for dedicated uninitialized-read jobs when the ecosystem permits it.
- **LSan** if leak detection is useful and not already covered via ASan integration.
- **SanitizerCoverage + libFuzzer/AFL/etc.** when fuzzing.
- **HWASan** on supported hardware where its tradeoffs make sense.
- **RTSan** only for code that actually has real-time constraints.

Trying to force everything into one build is usually wrong technically and operationally.

------

## 5. AddressSanitizer, in depth

### 5.1 What ASan detects

ASan is the workhorse. It detects many of the bugs C++ teams care about most:

- heap buffer overflows,
- stack buffer overflows,
- global buffer overflows,
- use-after-free,
- double-free in many cases,
- invalid frees,
- use-after-return / use-after-scope in supported modes,
- some invalid pointer comparisons/subtractions when enabled,
- and often leaks when LSan is integrated. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

Clang’s ASan docs explicitly describe compile and link usage, runtime requirements, extra checks, and limitations. GCC’s instrumentation docs note that `-fsanitize=address` also enables use-after-scope instrumentation and that additional invalid pointer-pair checks can be enabled through `ASAN_OPTIONS`. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

### 5.2 How ASan works conceptually

ASan uses **shadow memory** and **redzones**. The runtime poisons memory around allocations and deallocations so that accesses into those areas trap in the instrumented checks. Allocations are padded with guard regions. Frees often move blocks into a quarantine so stale pointers continue to fault instead of immediately being recycled. Stack and global objects are similarly instrumented with poisoned regions before/after them.

You do not need the exact encoding day to day, but the architectural idea matters because it explains:

- why ASan increases memory usage,
- why object layout changes,
- why some bugs become deterministic,
- why custom allocators or assembly can confuse it,
- and why “but it works without ASan” is meaningless.

### 5.3 Typical compile/link usage

Clang documents the canonical usage as:

```bash
clang++ -fsanitize=address -O1 -fno-omit-frame-pointer -g ...
```

and explicitly says the runtime must be linked into the final executable, so use the compiler driver for the final link, not `ld`. It also warns that when linking shared libraries, the ASan runtime is not linked there, and `-Wl,-z,defs` can cause link errors. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

GCC similarly documents `-fsanitize=address` and recommends using `-g` for useful output, with lower optimization and frame-pointer preservation improving stack traces. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))

MSVC enables ASan via `/fsanitize=address`; Microsoft documents support in Visual Studio 2019 16.9 and later and provides debugger/runtime integration specific to MSVC. It also defines `__SANITIZE_ADDRESS__` when enabled. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170))

### 5.4 ASan runtime options

ASan behavior is heavily tunable through `ASAN_OPTIONS`. GCC documents this generally, including `help=1`. Microsoft documents both `ASAN_OPTIONS` and `__asan_default_options()` with MSVC-specific behavior differences. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))

Common knobs you will see in practice include:

- symbolization behavior,
- leak detection behavior,
- allocation/deallocation mismatch handling,
- invalid pointer pair detection,
- initialization-order checking,
- quarantine sizing,
- exit/report behavior.

A specific example from Clang: dynamic initialization order checking can be enabled with `ASAN_OPTIONS=check_initialization_order=1`, but Clang notes that this option is not supported on macOS. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

### 5.5 ASan overhead and limitations

ASan is usually quite usable in CI and local repros, but not “free.” Clang notes higher real memory usage, higher stack memory usage, and large virtual address mappings on 64-bit systems. It also states that static linking of executables is not supported. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

Those details matter operationally:

- ASan builds often need more RAM in CI.
- `ulimit` expectations can be misleading because of large mapped virtual address ranges.
- Static-link-heavy environments can hit toolchain friction.
- Performance-sensitive integration tests may need separate sanitized job classes.

### 5.6 ASan strengths and weak spots

ASan is superb for classic memory corruption, but it is not omniscient.

It may miss:

- bugs in uninstrumented code,
- very low-level assembly,
- allocator misuse outside intercepted paths,
- temporal bugs that do not hit poisoned/quarantined regions,
- logic bugs that never violate a checked boundary,
- races, unless they manifest as memory corruption.

It can also surface latent UB that “worked” before instrumentation changed layout and timing.

### 5.7 ASan and custom allocators

If your C++ code uses custom pools, arenas, slab allocators, region allocators, intrusive memory schemes, or hand-written alloc/free wrappers, ASan may need extra care. The more your allocator bypasses the standard intercepted allocation APIs or manipulates raw virtual memory directly, the more likely you need explicit poisoning/unpoisoning hooks or to accept blind spots. On MSVC, Microsoft explicitly documents runtime interception and hooking custom allocators as part of ASan runtime guidance. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan-building?view=msvc-170))

Practical rule: if you own an allocator, test it in isolation under ASan and verify reports are meaningful before trusting coverage in higher layers.

------

## 6. LeakSanitizer, in depth

LSan detects leaked heap allocations at process end. Clang documents that it can be used together with ASan or standalone, and that it adds almost no overhead until the final leak-detection phase. GCC documents standalone `-fsanitize=leak` and notes it matters mainly at executable link time because the executable is linked with a library overriding allocator functions. ([Clang](https://clang.llvm.org/docs/LeakSanitizer.html))

In C++, LSan is especially useful for:

- ownership transitions during error handling,
- early returns,
- exception-heavy code,
- partially-constructed object graphs,
- caches with poorly-defined shutdown,
- tests that “pass” but leave garbage behind.

But there is an important engineering nuance: not all leaked allocations are bugs in the same sense.

Examples:

- intentionally process-lifetime caches,
- allocator singletons,
- global registries,
- lazily-initialized data intentionally not torn down.

So real teams often use suppressions or per-test leak policy instead of treating every leak as a release blocker.

LSan is a great correctness tool, but you need a leak philosophy.

------

## 7. UndefinedBehaviorSanitizer, in depth

### 7.1 What UBSan is

UBSan instruments code to catch many undefined behaviors or suspicious operations during execution. Clang describes it as a fast UB detector and lists examples such as statically-determinable array bounds issues, out-of-range bit shifts, null/misaligned pointer dereferences, and signed integer overflow. ([Clang](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html))

### 7.2 Why UBSan matters for C++

UB is where C++ gets terrifying. Once code executes UB, the optimizer may assume it never happened and transform the surrounding code aggressively. That means the visible symptom may be far removed from the source mistake.

UBSan shines for issues like:

- signed overflow,
- invalid shifts,
- null dereference assumptions,
- alignment violations,
- bad enum values,
- object-size issues,
- some bounds issues,
- vptr-related problems in polymorphic code,
- and other cases depending on enabled checks.

It is one of the best “cheap enough to run often” sanitizers.

### 7.3 Recovery vs trap mode

One of UBSan’s biggest advantages is configurability. Clang documents both recovery and trap behavior. You can let some checks continue so a single test run reports multiple bugs, force others to exit immediately, or put checks into trap mode. Clang also documents that with `-fsanitize-trap=all`, checks in trap mode do not require the UBSan runtime to be linked. GCC documents `-fsanitize-recover=all` / `-fno-sanitize-recover=all` too. ([Clang](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html))

This makes UBSan unusually adaptable:

- in CI, recovery can maximize bug yield per run;
- in debugger-focused local repros, immediate fail can be better;
- trap mode can be attractive for bare/minimal environments.

### 7.4 UBSan is not “proof against UB”

UBSan only checks what it instruments and what you execute. Some UB remains out of scope, some is architecture-dependent, and some requires other tools to expose. But if you do serious C++ work and are not running UBSan regularly, you are almost certainly leaving optimizer-sensitive bugs on the table.

------

## 8. ThreadSanitizer, in depth

### 8.1 What TSan detects

TSan detects **data races**: unsynchronized conflicting accesses to shared memory, where at least one access is a write and the accesses are not ordered by the language’s synchronization rules. Clang explicitly describes it as a data-race detector. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))

### 8.2 Why data races are special

A data race in C++ is not just “a threading bug.” It is UB. That means a race can yield values that seem impossible, produce optimizer-induced weirdness, or only fail under specific load and core topology.

TSan is often the only practical way to find real races in medium-to-large C++ systems.

### 8.3 Cost

TSan is expensive. Clang documents typical slowdown around **5x–15x** and memory overhead around **5x–10x**. It also documents additional memory costs per thread and large virtual address mappings. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))

That means you generally do not run the entire universe under TSan all the time. Instead:

- run focused unit/integration sets,
- run concurrency stress tests,
- run deterministic schedulers or workload repros,
- run smaller but high-contention scenarios.

### 8.4 Platform/build constraints

Clang documents TSan support on several 64-bit OS/architecture combinations and states that 32-bit support is problematic and not planned. It also notes that non-PIE executables are not supported and that Clang will effectively act as though `-fPIE`/`-pie` were supplied when needed. Static linking of libc/libstdc++ is also documented as unsupported. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))

These details are not trivia. They explain a large chunk of “why did our TSan build suddenly fail to link or deploy?”

### 8.5 TSan and atomics

TSan understands the C++ threading model, but “we used atomics” does not automatically mean “we are race-free.” It is still possible to misuse memory orders, rely on non-atomic side state, race through publication, or create higher-level synchronization bugs TSan may or may not classify the way you expect.

TSan is a data-race detector, not a general concurrent-correctness oracle.

### 8.6 TSan false positives?

TSan is much more trusted than many dynamic analyzers, but in real systems you will see reports that are:

- caused by custom synchronization TSan does not understand,
- caused by uninstrumented dependencies,
- rooted in third-party libraries,
- or theoretically benign but still races by the standard.

The last category is important. Many “benign races” are not benign in standard C++. They are just tolerated by your current platform.

------

## 9. MemorySanitizer, in depth

### 9.1 What MSan detects

MSan detects use of uninitialized memory. Clang says it reports cases such as uninitialized values used in conditional branches or uninitialized pointers used for memory accesses. It documents typical slowdown around **3x**. ([Clang](https://clang.llvm.org/docs/MemorySanitizer.html))

### 9.2 Why MSan is both amazing and painful

MSan can catch an entire class of bugs that ASan does not: code that reads indeterminate data but happens not to crash. This includes:

- conditionals on uninitialized flags,
- lengths derived from uninitialized bytes,
- partially-filled structs,
- unserialized padding,
- error paths using uninitialized locals,
- ABI crossings with partially-initialized aggregates.

For systems code, serializers, networking, parsing, and low-level C interop, MSan can be brutally effective.

But it is also operationally harder than ASan because it needs a large fraction of executed code, including dependencies, to be instrumented or otherwise correctly modeled. Uninstrumented libraries frequently become the reason MSan adoption stalls.

### 9.3 Platform and security notes

Clang documents supported platforms including Linux, NetBSD, and FreeBSD, and explicitly warns that MSan runtime is not meant for production executables and may compromise security constraints. ([Clang](https://clang.llvm.org/docs/MemorySanitizer.html))

So: MSan is a testing weapon, not a production hardening measure.

### 9.4 Best use cases

Use MSan when:

- you control most of the stack,
- you can rebuild dependencies,
- uninitialized data bugs are plausible,
- you are doing parsers, protocol stacks, codecs, numerics, serialization, or systems glue.

Do not make MSan your first sanitizer if your build graph is already fragile.

------

## 10. HWAddressSanitizer

HWASan is a hardware-assisted memory safety sanitizer, primarily targeting architectures like AArch64 with top-byte-ignore/tagging mechanisms. GCC documents it as AArch64-only and incompatible with `-fsanitize=thread` and `-fsanitize=address`. ([Clang](https://clang.llvm.org/docs/HardwareAssistedAddressSanitizerDesign.html?utm_source=chatgpt.com))

Conceptually, HWASan uses pointer tagging rather than the classic ASan shadow-memory approach alone, which can reduce overhead and improve some deployment characteristics on supported platforms.

In practice, HWASan matters most when:

- you target modern ARM environments,
- you want memory safety testing with different overhead/coverage tradeoffs,
- or you work in platforms where HWASan is the preferred sanitizer.

If your day job is mainstream desktop/server x86_64 C++ with Clang/GCC, classic ASan is still the default starting point.

------

## 11. TypeSanitizer

TypeSanitizer is niche but intellectually important for advanced C++ work. Clang describes it as detecting strict aliasing violations and explains that LLVM optimization uses TBAA metadata, which means invalid type punning can cause optimizers to introduce behavior changes that surprise developers. Clang also documents large memory overhead, with best-case slowdown around 2x–3x and memory overhead around 8x. ([Clang](https://clang.llvm.org/docs/TypeSanitizer.html))

This matters when you do:

- aggressive serialization/deserialization,
- packet views over byte buffers,
- JIT/runtime systems,
- embedded unions and reinterpretation,
- hand-rolled SIMD/layout tricks,
- or legacy code that relies on “works on my compiler” aliasing accidents.

If you have ever needed `-fno-strict-aliasing` as a safety blanket, TypeSanitizer is conceptually aimed right at that family of problems.

------

## 12. DataFlowSanitizer

DFSan is not a bug detector by itself. Clang documents it as a generalized dynamic data-flow analysis framework. You attach labels to data and observe where those labels propagate. ([Clang](https://clang.llvm.org/docs/DataFlowSanitizer.html?utm_source=chatgpt.com))

This is useful for:

- taint tracking,
- detecting whether untrusted input reaches dangerous sinks,
- verifying custom invariants,
- investigating confidentiality/integrity flows,
- application-specific security testing.

For most C++ application teams, DFSan is not a daily-driver tool. For security engineering, language runtimes, or custom dynamic analysis, it is extremely interesting.

------

## 13. RealtimeSanitizer

RTSan is one of the newer, more specialized sanitizers. Clang documents it as a runtime tool for detecting real-time safety violations in C/C++ code and says functions marked `[[clang::nonblocking]]` are treated as real-time functions. Clang further recommends using it together with compile-time Function Effect Analysis for broader coverage, and states the runtime slowdown is negligible. ([Clang](https://clang.llvm.org/docs/RealtimeSanitizer.html))

If you do:

- audio callbacks,
- rendering loops,
- hard timing pipelines,
- low-latency control code,

RTSan gives you something most sanitizer discussions ignore: not memory safety, but execution-context safety. For example, calling an allocating or blocking function from a callback that must not block.

It is specialized, but very relevant in the right domains.

------

## 14. SanitizerCoverage

SanitizerCoverage is instrumentation that inserts callbacks at function/basic-block/edge granularity. Clang explicitly notes that if you only want coverage visualization, source-based coverage may be the better tool, but SanitizerCoverage is low-overhead and often used alongside sanitizers. ([Clang](https://clang.llvm.org/docs/SanitizerCoverage.html))

Its biggest importance in practice is fuzzing. If you do coverage-guided fuzzing on a C++ library, SanitizerCoverage is one of the enabling technologies behind “the fuzzer keeps exploring new edges.”

For a C++ engineer, the practical triangle is:

- **SanitizerCoverage** to guide exploration,
- **ASan/UBSan** to turn weird inputs into immediate actionable failures,
- **a corpus/minimizer** to make those failures reproducible.

------

## 15. Clang vs GCC vs MSVC

### Clang

Clang has the richest sanitizer ecosystem and the most cohesive sanitizer documentation: ASan, UBSan, TSan, MSan, LSan, DFSan, TypeSanitizer, RealtimeSanitizer, SanitizerCoverage, ignorelists, feature macros, and related instrumentation. ([Clang](https://clang.llvm.org/docs/index.html?utm_source=chatgpt.com))

### GCC

GCC supports a substantial `-fsanitize=...` set and documents them under instrumentation options, including ASan, TSan, LSan, UBSan, HWASan, pointer-compare/subtract, recover controls, and more. The GCC docs are often less tutorial-like than Clang’s but still authoritative for GCC behavior. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))

### MSVC

Microsoft’s documentation centers on AddressSanitizer support via `/fsanitize=address`, runtime/debugger integration, and ASAN options/runtime configuration. That is the practical headline: if your organization is deeply Windows/MSVC-native, ASan is very usable, but the broader Clang sanitizer toolbox is not mirrored one-for-one in MSVC. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170))

### Practical recommendation

For maximum sanitizer depth in a cross-platform C++ shop:

- use **Clang** for your deepest sanitizer jobs,
- keep **GCC** sanitizer jobs where GCC-specific behavior matters,
- use **MSVC ASan** when you need first-class Windows/Visual Studio integration.

------

## 16. Can sanitizers be combined?

Sometimes.

Common patterns:

- **ASan + UBSan**: very common and useful.
- **ASan + LSan**: also common; LSan is often integrated with ASan. ([Clang](https://clang.llvm.org/docs/LeakSanitizer.html))
- **TSan** is generally separate from ASan and LSan. GCC explicitly documents that `-fsanitize=thread` cannot be combined with `address` or `leak`. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))
- **HWASan** is separate from ASan and TSan. GCC documents this explicitly. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))
- **UBSan trap/recover modes** can coexist with other sanitizer configurations, but exact behavior depends on toolchain and enabled checks. ([Clang](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html))

General rule: combine where the toolchains and runtimes are designed to cooperate; otherwise keep builds separate.

A healthy sanitizer CI usually has multiple jobs, not a single monster configuration.

------

## 17. Compile, link, and optimize correctly

Sanitizers are build-configuration-sensitive.

For Clang ASan and MSan, the docs explicitly say to use the compiler driver for the final link because the runtime must be linked to the executable. Clang also warns that certain linker options such as `-Wl,-z,defs` can fail for sanitized shared-library builds because the runtime is not linked there. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

For useful reports, common guidance includes:

- `-g`
- `-fno-omit-frame-pointer`
- modest optimization like `-O1` or `-Og`
- sometimes disabling sibling-call optimization or aggressive inlining when stack quality matters

Clang and GCC both document parts of this guidance directly for ASan and MSan. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))

Important nuance: sanitizers do **not** require `-O0`. In fact, `-O0` can make programs too different from realistic optimized code. A lot of teams prefer `-O1` for sanitizer builds because it preserves enough structure for debugging while still resembling optimized execution more closely.

------

## 18. Feature detection in source code

Clang documents feature-test support through `__has_feature(...)`, including:

- `__has_feature(address_sanitizer)`
- `__has_feature(thread_sanitizer)`
- `__has_feature(memory_sanitizer)`
- `__has_feature(dataflow_sanitizer)` ([Clang](https://clang.llvm.org/docs/LanguageExtensions.html))

MSVC documents `__SANITIZE_ADDRESS__` for ASan-enabled builds. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan-building?view=msvc-170))

This is useful when you need conditional code such as:

- suppressing unsupported tricks,
- adding extra diagnostics,
- changing allocator behavior,
- or guarding instrumentation-specific hooks.

But use this sparingly. Do not litter the codebase with sanitizer-specific logic unless you are isolating well-understood low-level exceptions.

------

## 19. Suppressions, ignorelists, and selective disabling

No serious codebase uses sanitizers at scale without suppression mechanisms.

Clang documents the **Sanitizer special case list**, which lets users disable or alter sanitizer behavior for certain source-level entities via a compile-time file. The docs explicitly mention common reasons: speeding up a hot function known to be correct, or excluding low-level magic that intentionally violates normal assumptions. ([Clang](https://clang.llvm.org/docs/SanitizerSpecialCaseList.html))

This is the right way to handle:

- assembly thunks,
- stack-walking code,
- coroutine/runtime internals,
- platform shims,
- known-safe but tool-hostile allocator paths,
- temporary third-party exclusions.

There are also source-level attributes such as `no_sanitize(...)` in Clang/GCC ecosystems, but they should be used like `unsafe` in other languages: deliberately, locally, and with comments explaining why.

The failure mode to avoid is “we suppressed the report because it was annoying.” Suppress only when you can articulate why the tool is wrong for that region, not why the report is inconvenient.

------

## 20. Why sanitized builds behave differently

This is normal. Instrumentation changes:

- object layout,
- allocator behavior,
- alignment,
- stack frames,
- timing,
- inlining,
- memory reuse patterns,
- thread scheduling windows,
- crash timing,
- and optimization opportunities.

So you will see bugs that disappear under sanitizers and bugs that appear only under sanitizers.

That does **not** make sanitizers unreliable. It is the opposite. It means your program depends on undefined or fragile behavior, and the tool changed the environment enough to reveal it.

For seasoned C++ engineers, this is one of the hardest but most important lessons: “ASan made it crash” often really means “ASan stopped your bug from silently corrupting memory.”

------

## 21. False positives, false negatives, and trust

Sanitizers are high-value precisely because they usually have **low false-positive rates** for the things they claim to detect. Microsoft’s ASan documentation even emphasizes zero false positives for the class of bugs it targets. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170))

That said, you should think in three buckets.

### True positive

The best case. Fix it.

### Tooling mismatch / modeling gap

Examples:

- custom allocator not understood,
- custom synchronization not understood,
- dependency not instrumented,
- assembly boundary confuses origin tracking.

These may look like false positives, but often the right answer is “improve the instrumentation boundary.”

### False negative

Common causes:

- code path not executed,
- uninstrumented module,
- UB outside the sanitizer’s scope,
- optimization/transformation hides a dynamic manifestation,
- bug class belongs to another sanitizer.

Practical rule: treat sanitizer findings as highly credible, but treat sanitizer cleanliness as very far from proof of correctness.

------

## 22. Sanitizers and the C++ object model

Sanitizers get especially interesting around the C++ object model.

### Lifetime

ASan catches many lifetime violations once they manifest as bad memory accesses. UBSan and vptr-related checks can help with dynamic type misuse. LSan catches lost ownership that never frees.

### Initialization

MSan is the main weapon for indeterminate values. In modern C++, many of these are caused by partially initialized aggregates, error paths, and interactions with C APIs.

### Aliasing

TypeSanitizer exists because C++ aliasing rules are stricter than many programmers internalize. Reinterpretation tricks that “worked for years” can break under optimization. ([Clang](https://clang.llvm.org/docs/TypeSanitizer.html))

### Concurrency

TSan is the main runtime expression of the C++ memory model in tooling. If you write lock-free or lock-light code, it is indispensable.

### Exceptions and error paths

Sanitizers are disproportionately good at finding bugs hidden in unusual control flow: unwinding, cancellation, partial construction, early returns, and failure cleanup.

------

## 23. Sanitizers and STL-heavy C++

C++ teams sometimes assume RAII and STL eliminate the need for sanitizers. They do not.

Sanitizers routinely find:

- iterator invalidation misuse,
- stale references into `std::vector` after reallocation,
- dangling `std::string_view`,
- lifetime bugs crossing `std::span`,
- use-after-move that turns into invalid access later,
- `std::optional`/`variant` state misuse leading to UB,
- data races on shared containers,
- leaks hidden behind `shared_ptr` cycles or caches,
- padding/uninitialized bytes in POD-ish structs passed to C or hashing code.

Modern C++ removes some categories of errors. It also creates new ways to express them cleanly enough that code review misses them.

------

## 24. Sanitizers and low-level/networking/systems C++

Since you are a C++ software engineer, and likely touch systems-level code, this is especially relevant.

Sanitizers are extremely effective for:

- packet parsing,
- binary protocols,
- socket buffer handling,
- ring buffers,
- intrusive data structures,
- DMA-ish or mapped memory interfaces,
- message serialization,
- custom small-buffer optimizations,
- coroutine state/lifetime edges,
- allocator-backed transport queues.

They are less naturally effective where a lot of logic happens in:

- kernel space,
- inline assembly,
- syscalls with raw shared memory semantics,
- JIT-emitted code,
- heavily custom runtime systems,
- or performance-critical code full of intentional low-level tricks.

That does not mean “don’t use sanitizers” there. It means expect to add annotations, targeted suppressions, and focused tests.

------

## 25. Production use vs test use

Sanitizers are primarily **testing** tools.

Clang explicitly says MSan’s runtime is not intended for production executables and may compromise security-sensitive assumptions. That is representative of the broader sanitizer philosophy. ([Clang](https://clang.llvm.org/docs/MemorySanitizer.html))

Some organizations do deploy ASan/HWASan in staging, canary, dogfood, or internal builds. That can be incredibly effective. But full production deployment is usually constrained by:

- performance overhead,
- memory overhead,
- binary size,
- runtime library expectations,
- security model concerns,
- and incompatible platform packaging/linking requirements.

So the normal maturity ladder is:

1. local developer runs,
2. sanitizer CI,
3. nightly/integration sanitizer suites,
4. fuzzing sanitizer jobs,
5. optional staging/canary instrumentation.

------

## 26. CI strategy that actually works

A practical sanitizer matrix for a C++ team looks like this:

### Fast per-PR job

- Clang or GCC
- `ASan + UBSan`
- medium-sized unit tests
- frame pointers on
- symbols on

### Concurrency job

- Clang
- `TSan`
- reduced but contention-heavy tests
- stress loops/repeated runs

### Uninitialized-value job

- Clang
- `MSan`
- only if dependency rebuild story is realistic

### Leak-focused job

- `ASan` with leak detection or standalone `LSan`
- teardown-sensitive integration tests

### Fuzzing

- `SanitizerCoverage + ASan + UBSan`
- persistent corpus
- crash minimization

### Optional architecture-specific

- `HWASan` on AArch64
- `RTSan` on real-time code paths

The biggest anti-pattern is one gigantic sanitized CI job that is too slow, too flaky, and therefore ignored.

------

## 27. How to triage sanitizer reports

When a sanitizer fires:

1. Read the **first** report carefully. Later crashes may be fallout.
2. Identify the exact access or operation.
3. Find the allocation/free/origin stacks if present.
4. Reproduce with the smallest test or input possible.
5. Fix the root cause, not the symptom.
6. Re-run under the same sanitizer.
7. Re-run under adjacent sanitizers when relevant.

For example:

- ASan hit in a parser? Also run UBSan.
- TSan hit in a queue? Also run ASan because races often mask memory corruption.
- Leak hit in an exception path? Also run UBSan/MSan depending on context.

The deeper point is that sanitizer findings often come in families.

------

## 28. Common mistakes teams make

### “We ran ASan once.”

That is not a process.

### “No sanitizer reports means memory-safe.”

No. It means no instrumented failure was observed on executed paths.

### “TSan says race, but it’s benign.”

Usually not benign by the standard.

### “We use smart pointers, so ASan won’t help.”

It will.

### “MSan is too hard, so sanitizers are not worth it.”

ASan and UBSan alone usually pay for themselves quickly.

### “We should disable sanitizer around all hot code.”

No. The right question is whether the code is both hot and well-understood enough to justify selective exclusion.

### “Sanitizer build should mirror release flags exactly.”

Not entirely. You want realistic optimization, but also debuggability and report quality.

------

## 29. Sanitizers and fuzzing

This is one of the highest-ROI combinations in native code.

Coverage-guided fuzzing plus sanitizers turns weird edge-case inputs into deterministic, high-signal bug reports. SanitizerCoverage gives the fuzzer path guidance, and ASan/UBSan convert latent corruption or UB into immediate failures. Clang’s docs explicitly position SanitizerCoverage as low-overhead instrumentation and note source-based coverage as a separate alternative when plain coverage reporting is the goal. ([Clang](https://clang.llvm.org/docs/SanitizerCoverage.html))

For libraries, parsers, codecs, protocol handlers, file readers, and serialization frameworks, this is often the best bug-finding setup you can build.

------

## 30. Sanitizers and third-party libraries

A sanitized executable is only as complete as the instrumented world it actually executes through.

Problems arise when:

- your main binary is sanitized but dependencies are not,
- plugins are built with different flags,
- static libs were built unsafely,
- system libs lack expected interceptability,
- C libraries pass partially-initialized structs,
- custom allocators or custom synchronization hide semantics.

This hurts MSan the most, but all sanitizers can be affected.

Best practice:

- sanitize what you own first,
- rebuild important internal dependencies with matching flags,
- maintain suppressions for the rest,
- do not block adoption on perfect coverage.

------

## 31. Special notes for Windows users

MSVC ASan is real, useful, and worth using on Windows-native C++ code. Microsoft documents `/fsanitize=address`, debugger integration, runtime interception, `__SANITIZE_ADDRESS__`, `ASAN_OPTIONS`, and even crash-dump workflows for ASan failures. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170))

For Windows-centric shops, the practical pattern is often:

- use MSVC ASan for day-to-day IDE-centric workflows,
- use Clang-cl or Clang-based builds for deeper sanitizer coverage when needed,
- keep symbolization and runtime configuration disciplined across CI and local machines.

------

## 32. Important limitations you should internalize

Here are the most important ones.

### Sanitizers do not prove absence of bugs

They prove presence of observed instrumented failures.

### Unexecuted code is invisible

No dynamic execution, no finding.

### Uninstrumented code is a blind spot

Especially serious for MSan and TSan.

### Some sanitizer builds need platform concessions

Examples: PIE requirements for TSan, AArch64-only HWASan, unsupported static linking cases for ASan/TSan. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))

### Reports can be secondary explosions

The first invalid operation is the one that matters.

### Sanitizers can perturb timing/layout

This is not a bug in the sanitizer.

------

## 33. Recommended starting setup for a serious C++ project

If I were designing a sane default sanitizer strategy for a modern C++ codebase, I would start here:

### Developer local build

Clang:

```bash
-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined
```

### PR CI

Same as above, moderate test subset.

### Nightly

- broader `ASan + UBSan`
- dedicated `TSan` job
- leak-detection-enabled run
- fuzz targets under `ASan + UBSan + SanitizerCoverage`

### Optional advanced

- `MSan` for subsystems with rebuildable deps
- `HWASan` on supported ARM infrastructure
- `RTSan` for actual real-time code paths

That setup catches a huge fraction of native-code bugs at manageable cost.

------

## 34. Which sanitizer should you use for which symptom?

Use **ASan** when you see:

- segfaults,
- heap corruption,
- weird iterator/reference crashes,
- “crash much later than the bug.”

Use **UBSan** when you see:

- optimization-only weirdness,
- signed overflow suspicion,
- invalid casts or alignment issues,
- impossible control-flow behavior.

Use **TSan** when you see:

- nondeterministic test failures,
- state corruption only under load,
- bugs that disappear with logging,
- lock-free or weakly synchronized code.

Use **MSan** when you see:

- random branch choices,
- nondeterministic protocol fields,
- “garbage” values,
- partially initialized structs crossing boundaries.

Use **LSan** when you see:

- steadily increasing memory in tests,
- ownership cleanup uncertainty,
- exception/error path leaks.

Use **TypeSanitizer** when you suspect:

- aliasing tricks,
- type punning,
- “works unless optimized.”

Use **RTSan** when you suspect:

- forbidden allocations or blocking calls in real-time contexts.

------

## 35. The deeper engineering lesson

Sanitizers are not just debugging aids. They are executable specifications for parts of the C/C++ abstract machine that the language itself leaves dangerously permissive.

Each sanitizer says, in effect:

- “this memory must still be alive,”
- “this access must stay within object boundaries,”
- “this value must be initialized before it influences control flow,”
- “these accesses must be synchronized,”
- “this real-time callback must not allocate or block,”
- “this aliasing pattern must obey the optimizer’s assumptions.”

That is why seasoned C++ engineers who adopt sanitizers seriously usually never go back. They make the language’s sharpest edges visible.

------

## 36. The shortest practical advice I can give

If you do only four things, do these:

1. Run **ASan + UBSan** constantly.
2. Run **TSan** in a dedicated CI lane.
3. Add **fuzzing with SanitizerCoverage + ASan/UBSan** for parsers and protocol code.
4. Treat sanitizer findings as design feedback, not just bug tickets.

That alone will change the quality curve of a C++ codebase.

------

## 37. Authoritative current references

For current behavior, flags, and platform specifics, these are the primary sources I grounded this on:

- Clang AddressSanitizer docs. ([Clang](https://clang.llvm.org/docs/AddressSanitizer.html))
- Clang UndefinedBehaviorSanitizer docs. ([Clang](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html))
- Clang ThreadSanitizer docs. ([Clang](https://clang.llvm.org/docs/ThreadSanitizer.html))
- Clang MemorySanitizer docs. ([Clang](https://clang.llvm.org/docs/MemorySanitizer.html))
- Clang LeakSanitizer docs. ([Clang](https://clang.llvm.org/docs/LeakSanitizer.html))
- Clang SanitizerCoverage docs. ([Clang](https://clang.llvm.org/docs/SanitizerCoverage.html))
- Clang TypeSanitizer docs. ([Clang](https://clang.llvm.org/docs/TypeSanitizer.html))
- Clang RealtimeSanitizer docs. ([Clang](https://clang.llvm.org/docs/RealtimeSanitizer.html))
- Clang feature-test and ignorelist docs. ([Clang](https://clang.llvm.org/docs/LanguageExtensions.html))
- GCC instrumentation options. ([GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html))
- Microsoft ASan docs, build/runtime/options. ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170))

I can also turn this into a second part focused specifically on **sanitizers for modern C++ codebases**: CMake presets, recommended flags, CI matrices, suppressions, symbolization, common STL/container bugs, coroutines, custom allocators, and examples of real ASan/TSan/UBSan findings in idiomatic C++20 code.

---

# The Complete C/C++ Sanitizers Handbook, Part II: Sanitizers in Modern C++ Codebases

This second part is about using sanitizers well in a real C++20 codebase, not just knowing what they are. The focus here is build architecture, CMake integration, CI strategy, suppressions, symbolization, and the places where sanitizers intersect with idiomatic modern C++: STL, `string_view`, `span`, coroutines, allocators, atomics, and ownership-heavy code.

The short version of the operational model is:

* Standardize sanitized builds with **CMake presets**.
* Put sanitizer flags on **targets**, not globally, whenever practical.
* Treat **ASan + UBSan** as your default checked build.
* Run **TSan** in a separate lane.
* Run **MSan** only where you can rebuild enough of the dependency stack.
* Use **ignorelists and targeted `no_sanitize` escapes** sparingly and deliberately.
* Make **symbolization** reliable, or sanitizer reports will be much less useful.
* Expect sanitizers to expose bugs that look like STL bugs but are actually lifetime, invalidation, or concurrency bugs in your code. CMake presets are supported via `CMakePresets.json` and `CMakeUserPresets.json`; presets were added in CMake 3.19, and the current format supports shared project presets plus local-user overrides. CMake also recommends target-level options and supports per-language flag selection via generator expressions. ([CMake][1])

---

## 1. How modern C++ teams should think about sanitizer builds

In a healthy C++ codebase, sanitizer support is not one boolean like `ENABLE_SANITIZERS=ON`. It is a build matrix.

You generally want these build personalities:

* **regular debug** for interactive development,
* **asan-ubsan** for day-to-day checked execution,
* **tsan** for race-focused testing,
* **msan** for uninitialized-read hunting where feasible,
* optionally **fuzz** with SanitizerCoverage plus ASan/UBSan,
* optionally **release-with-asan** for staging repros.

The reason is technical, not organizational. ASan is relatively affordable and catches a large class of bugs with about 2x slowdown; UBSan has small runtime cost and no ABI or address-space-layout impact; TSan is much heavier, with roughly 5x–15x slowdown and 5x–10x memory overhead; MSan has its own symbolization and dependency constraints. Trying to collapse those into one build tends to produce a miserable build nobody uses. ([Clang][2])

For modern C++ teams, the most useful default is:

* **local developer checked build**: ASan + UBSan,
* **PR CI**: ASan + UBSan,
* **nightly or dedicated lane**: TSan,
* **specialized lane**: MSan if your ecosystem supports it.

---

## 2. CMake architecture: do not smear sanitizer flags across the whole build by accident

The most common CMake mistake is shoving sanitizer flags into `CMAKE_CXX_FLAGS` or some global cache variable and hoping for the best.

That can work, but it scales poorly because:

* it affects everything, including third-party code,
* it makes partial instrumentation difficult,
* it complicates host tools or code generators built as part of the same tree,
* it is hard to reason about when some targets should not be instrumented.

CMake’s docs explicitly steer you toward target-level commands such as `target_compile_options()` and `target_link_options()`, and note that `target_compile_options()` supports per-language control through the `COMPILE_LANGUAGE` generator expression. ([CMake][3])

A better pattern is to define a reusable interface target.

### Example: interface target for sanitizers

```cmake
add_library(project_sanitizers INTERFACE)

target_compile_options(project_sanitizers INTERFACE
  $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer>
  $<$<COMPILE_LANGUAGE:C,CXX>:-g>
)

target_link_options(project_sanitizers INTERFACE)
```

Then attach sanitizer-specific options conditionally.

### Example: ASan + UBSan on Clang/GCC

```cmake
option(ENABLE_ASAN_UBSAN "Enable ASan+UBSan" OFF)

if(ENABLE_ASAN_UBSAN)
  target_compile_options(project_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
    $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,undefined>
    $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer>
  )

  target_link_options(project_sanitizers INTERFACE
    -fsanitize=address,undefined
  )
endif()
```

Then apply it only where you mean it:

```cmake
target_link_libraries(my_lib PRIVATE project_sanitizers)
target_link_libraries(my_tests PRIVATE project_sanitizers)
```

That gives you control. Your code generators, benchmark tools, or external vendored code do not need to be dragged into the same instrumentation regime unless you choose it.

---

## 3. CMake presets: the cleanest way to standardize sanitized builds

CMake presets are the right place to define shared sanitizer build personalities for the whole team. CMake documents `CMakePresets.json` as the project-wide shared file and `CMakeUserPresets.json` as the local-user override file that should not be checked in. Presets were added in CMake 3.19, and current docs show the modern schema and inheritance model. ([CMake][1])

### A practical `CMakePresets.json`

```json
{
  "version": 10,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 23,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "base-ninja",
      "displayName": "Base Ninja",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "BUILD_TESTING": "ON"
      }
    },
    {
      "name": "asan-ubsan",
      "inherits": "base-ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN_UBSAN": "ON"
      }
    },
    {
      "name": "tsan",
      "inherits": "base-ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_TSAN": "ON"
      }
    },
    {
      "name": "msan",
      "inherits": "base-ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_MSAN": "ON"
      }
    }
  ],
  "buildPresets": [
    { "name": "asan-ubsan", "configurePreset": "asan-ubsan" },
    { "name": "tsan", "configurePreset": "tsan" },
    { "name": "msan", "configurePreset": "msan" }
  ],
  "testPresets": [
    {
      "name": "asan-ubsan",
      "configurePreset": "asan-ubsan",
      "output": { "outputOnFailure": true }
    },
    {
      "name": "tsan",
      "configurePreset": "tsan",
      "output": { "outputOnFailure": true }
    }
  ]
}
```

This gives every developer and CI runner the same entry points:

```bash
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan
```

The win here is cultural as much as technical: no wiki page of magical flags, no shell-script drift, no “works on my laptop” preset mismatch.

---

## 4. Recommended sanitizer flag sets for modern C++

### 4.1 Clang/GCC: default checked build

For most C++20 projects, the best default checked build is:

```bash
-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined
```

Clang’s ASan docs explicitly recommend `-O1` or higher for reasonable performance and `-fno-omit-frame-pointer` for nicer stack traces, and note that compile and final link both need the sanitizer enabled through the compiler driver. Clang’s UBSan docs document the runtime and low-overhead nature of many checks. ([Clang][2])

### 4.2 Useful UBSan refinement

Many teams do not want the broadest possible `undefined` group without thought. A common practical profile is to start with:

```bash
-fsanitize=undefined
-fno-sanitize=alignment
```

or similar selective adjustment when a codebase has legacy alignment assumptions. That is not because alignment UB is okay. It is because you want adoption without immediately drowning in legacy failures. UBSan supports selective check control and recovery/trap behavior. ([Clang][4])

### 4.3 TSan build

```bash
-O1 -g -fno-omit-frame-pointer -fsanitize=thread
```

TSan is separate because of runtime and compatibility constraints. Clang documents TSan as a dedicated runtime with major slowdown/memory overhead and support focused on 64-bit platforms. ([Clang][5])

### 4.4 MSan build

```bash
-O1 -g -fno-omit-frame-pointer -fsanitize=memory
```

Often with:

```bash
-fsanitize-memory-track-origins=2
```

Clang documents origin tracking via `-fsanitize-memory-track-origins` and symbolization via `llvm-symbolizer` or `MSAN_SYMBOLIZER_PATH`. ([Clang][6])

### 4.5 Clang-specific ASan extras worth knowing

Clang ASan supports additional checks such as:

* `-fsanitize-address-use-after-scope`
* `-fsanitize-address-use-after-return=(never|runtime|always)`

and the runtime option `ASAN_OPTIONS=detect_stack_use_after_return=1`, already enabled on Linux according to the docs. These are especially relevant to modern C++ because so many lifetime bugs are stack/lambda/view-related. ([Clang][2])

---

## 5. A complete CMake module pattern

Here is a practical CMake pattern I would actually use.

```cmake
option(ENABLE_ASAN_UBSAN "Enable ASan + UBSan" OFF)
option(ENABLE_TSAN "Enable TSan" OFF)
option(ENABLE_MSAN "Enable MSan" OFF)

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)
add_library(project_sanitizers INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)

if (MSVC)
  if (ENABLE_ASAN_UBSAN)
    target_compile_options(project_sanitizers INTERFACE /fsanitize=address /Zi)
    target_link_options(project_sanitizers INTERFACE /INCREMENTAL:NO)
  endif()
else()
  target_compile_options(project_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:C,CXX>:-g>
    $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer>
  )

  if (ENABLE_ASAN_UBSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,undefined>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize-address-use-after-scope>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=address,undefined
    )
  endif()

  if (ENABLE_TSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=thread>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=thread
    )
  endif()

  if (ENABLE_MSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=memory>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize-memory-track-origins=2>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=memory
    )
  endif()
endif()
```

And then:

```cmake
target_link_libraries(mycore PRIVATE project_options project_warnings project_sanitizers)
target_link_libraries(mytests PRIVATE project_options project_warnings project_sanitizers)
```

This pattern scales nicely across applications, libraries, test binaries, and examples.

---

## 6. Symbolization: without it, sanitizer reports are half-blind

A sanitizer report without good symbols is much less useful than people admit.

Clang’s ASan and MSan docs explicitly call out symbolization requirements. ASan uses `llvm-symbolizer`, and MSan says to ensure `llvm-symbolizer` is in `PATH` or set `MSAN_SYMBOLIZER_PATH`. On macOS, Clang notes you may need `dsymutil` to get file:line information. ([Clang][2])

### Practical Linux/macOS setup

```bash
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
export MSAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
```

and compile with:

```bash
-g -fno-omit-frame-pointer
```

For macOS, also generate debug info as needed for symbol tools.

### Windows/MSVC

Microsoft documents Visual Studio integration for ASan and support for crash dumps with `ASAN_SAVE_DUMPS=...`, which is especially useful in distributed CI or remote repro environments. ([Microsoft Learn][7])

### Practical advice

Do not treat symbolization as “nice to have.” Make it part of the preset or CI environment.

If your CI emits raw addresses and mangled stack fragments, developers will subconsciously stop trusting sanitizer output because triage becomes expensive.

---

## 7. Suppressions, ignorelists, and `no_sanitize`: how to use them without lying to yourself

Clang documents the **Sanitizer special case list**, often called the ignorelist, for disabling or altering sanitizer behavior for certain entities at compile time. ASan, UBSan, TSan, MSan, and TypeSanitizer all document ignorelist support, though entity kinds differ somewhat. ([Clang][8])

### Example ignorelist file

```text
# sanitizer_ignorelist.txt

# Ignore a specific source file
src:third_party/legacy_allocator.cpp

# Ignore a hot low-level function
fun:myproject::detail::fast_memcpy

# ASan-specific global/type support exists too
global:legacy_global_buffer
type:myproject::PackedWireHeader
```

Clang documents that ASan supports `src` and `fun`, and additionally `global` and `type` for some global out-of-bounds suppressions. UBSan and TSan support `src` and `fun`. ([Clang][9])

Then pass it during compilation, typically with the appropriate ignorelist flag for the compiler/runtime configuration you use.

### Function-level exclusions

Clang/GCC support source attributes such as `no_sanitize_address`, `no_sanitize_thread`, and `no_sanitize_undefined` depending on compiler and sanitizer. GCC documents `no_sanitize_address` and related attributes in its common attributes docs. Clang documents feature-test macros like `__has_feature(address_sanitizer)`. MSVC documents `__declspec(no_sanitize_address)` and states that it disables compiler behavior, not runtime behavior. ([Clang][10])

### Example portable-ish macro

```cpp
#if defined(_MSC_VER) && defined(__SANITIZE_ADDRESS__)
  #define NO_ASAN __declspec(no_sanitize_address)
#elif defined(__clang__) || defined(__GNUC__)
  #define NO_ASAN __attribute__((no_sanitize_address))
#else
  #define NO_ASAN
#endif
```

Use this only for truly low-level code that the sanitizer fundamentally cannot model cleanly, such as:

* custom context switching,
* unusual stack tricks,
* special allocator primitives,
* ABI shims,
* hand-written asm wrappers.

Do not use it just because a test started failing.

---

## 8. A realistic CI matrix

A modern C++ codebase should not have one “sanitizer job.” It should have a few jobs with clear purposes.

### Per-PR

* Linux Clang `asan-ubsan`
* moderate unit and integration suite
* fail on first sanitizer finding

### Nightly

* Linux Clang `tsan`
* race-prone tests, stress loops, repetition
* possibly smaller subset, run multiple times

### Optional nightly/weekly

* Linux Clang `msan`
* only subsystems with instrumentable dependencies
* origin tracking on

### Windows

* MSVC ASan
* IDE/debugger-friendly build
* crash dump collection in CI if needed

### Fuzzing

* Clang with SanitizerCoverage + ASan/UBSan
* persistent corpus and crash retention

This maps directly onto tool behavior: ASan is affordable enough to use routinely; UBSan is cheap; TSan is expensive and separate; MSan has ecosystem friction; SanitizerCoverage is low-overhead instrumentation designed for coverage-driven workflows. ([Clang][2])

---

## 9. Common sanitizer findings in idiomatic C++20

This is the part many teams underestimate. Modern C++ does not eliminate sanitizer findings. It changes their shape.

### 9.1 Dangling `std::string_view`

```cpp
#include <string>
#include <string_view>

std::string_view make_view() {
    return std::string("temporary");
}
```

This is a lifetime bug. `string_view` does not own storage. ASan may report a stack or heap use-after-free later, depending on how the temporary was materialized and used.

More realistic:

```cpp
std::string_view first_token() {
    std::string line = read_line();
    auto pos = line.find(' ');
    return std::string_view(line.data(), pos);
}
```

It looks cheap and modern. It is still dangling.

### 9.2 Dangling `std::span`

```cpp
#include <span>
#include <vector>

std::span<int> bad() {
    std::vector<int> v = {1,2,3};
    return std::span<int>(v);
}
```

Again, `span` is non-owning. ASan often catches the eventual access, not the return itself.

### 9.3 Iterator invalidation after growth

```cpp
std::vector<int> v{1,2,3};
auto it = v.begin();
v.push_back(4);   // may reallocate
int x = *it;      // ASan often catches this
```

This is one of the most common “the STL crashed” bugs that sanitizers help pin down correctly.

### 9.4 Reference invalidation into `std::vector`

```cpp
auto& ref = v[0];
v.reserve(1000);   // may move storage
use(ref);          // dangling reference
```

In large codebases, this often hides across helper layers and looks like random corruption much later.

### 9.5 Use-after-move that becomes memory misuse later

Use-after-move is not inherently UB for all types, but code frequently assumes moved-from objects retain stronger invariants than they actually do.

```cpp
std::string s = "payload";
auto t = std::move(s);
send_raw(s.data(), s.size()); // logic bug; may later become UB depending on assumptions
```

Sanitizers may not fire on the move itself, but they often fire when stale views or raw pointers derived before or after the move are used incorrectly.

---

## 10. ASan examples in real modern C++ style

### 10.1 Returning a view into destroyed storage

```cpp
#include <string>
#include <string_view>

std::string_view get_user_prefix() {
    std::string user = "alice@example.com";
    return std::string_view(user.data(), 5);
}

int main() {
    auto v = get_user_prefix();
    return v[0];
}
```

Likely sanitizer story:

* no complaint at return,
* ASan complaint on `v[0]`,
* report points to use-after-return/use-after-scope depending on platform/build/runtime settings. Clang documents dedicated ASan support for use-after-return and use-after-scope detection. ([Clang][2])

### 10.2 `std::optional<std::reference_wrapper<T>>` into erased element

```cpp
#include <functional>
#include <optional>
#include <vector>

std::optional<std::reference_wrapper<int>> find_first_even(std::vector<int>& v) {
    for (auto& x : v) {
        if ((x % 2) == 0) return x;
    }
    return std::nullopt;
}

int main() {
    std::vector<int> v{1, 2, 3};
    auto r = find_first_even(v);
    v.push_back(4); // maybe reallocate
    return r->get(); // may become dangling
}
```

The code looks “safe” because it uses modern vocabulary types. The reference still dangles.

### 10.3 Lambda captures raw pointer to object that dies

```cpp
#include <functional>
#include <memory>

std::function<int()> make_task() {
    auto p = std::make_unique<int>(42);
    int* raw = p.get();
    return [raw] { return *raw; };
}

int main() {
    auto f = make_task();
    return f(); // use-after-free
}
```

ASan catches this class of ownership bug extremely well.

---

## 11. UBSan examples in idiomatic C++20

### 11.1 Signed overflow in “harmless” arithmetic

```cpp
#include <cstdint>

int grow(int n) {
    return n * 2;
}
```

If `n` is near `INT_MAX / 2`, UBSan can flag signed integer overflow. Clang documents signed integer overflow among the checks in UBSan. ([Clang][4])

In real C++ code this often appears in:

* buffer growth policies,
* serialization length math,
* retry backoff,
* custom hash mixing,
* calendar/time arithmetic.

### 11.2 Invalid shift in bit-manipulation utility

```cpp
#include <cstdint>

std::uint32_t mask(std::uint32_t bits) {
    return 1u << bits; // UB if bits >= 32
}
```

Again, UBSan is built for exactly this.

### 11.3 Misaligned reinterpretation in packet code

```cpp
#include <cstdint>
#include <span>

std::uint32_t read_u32(std::span<const std::byte> s) {
    auto p = reinterpret_cast<const std::uint32_t*>(s.data() + 1);
    return *p; // possible misaligned access
}
```

Clang documents misaligned pointer dereference checks in UBSan. This one shows up all the time in networking and binary formats. ([Clang][4])

### 11.4 Enum state corruption

```cpp
enum class State : unsigned char { Idle, Running, Done };

State decode(unsigned char x) {
    return static_cast<State>(x);
}
```

Depending on enabled UBSan checks and downstream use, invalid enum values can be surfaced dynamically.

---

## 12. TSan examples in modern C++

### 12.1 “It’s just a flag”

```cpp
#include <thread>

bool done = false;

int main() {
    std::thread t([] {
        while (!done) {}
    });

    done = true;
    t.join();
}
```

This is a race. In real code it often hides behind task loops and shutdown flags.

### 12.2 Shared `std::vector` without synchronization

```cpp
#include <thread>
#include <vector>

std::vector<int> data;

int main() {
    std::thread t1([] { data.push_back(1); });
    std::thread t2([] { data.push_back(2); });
    t1.join();
    t2.join();
}
```

People sometimes expect STL containers to “handle this.” They do not. TSan is designed to catch exactly these unsynchronized shared accesses. Clang documents TSan as a runtime data-race detector. ([Clang][5])

### 12.3 `shared_ptr` misuse misunderstanding

The control block’s reference counting is thread-safe, but the pointee is not magically synchronized.

```cpp
#include <memory>
#include <thread>

struct Counter {
    int value = 0;
};

int main() {
    auto p = std::make_shared<Counter>();

    std::thread t1([p] { ++p->value; });
    std::thread t2([p] { ++p->value; });

    t1.join();
    t2.join();
}
```

TSan should flag the race on `value`. This is one of the most common “but we used `shared_ptr`” misconceptions.

---

## 13. Coroutines: where sanitizers become extremely valuable

Modern C++ coroutines create a new space for lifetime errors, especially around frame ownership, borrowed references, and resumptions after object destruction.

### 13.1 Dangling reference captured into coroutine frame

```cpp
task<int> f() {
    std::string s = "hello";
    co_await something();
    co_return s.size();
}
```

This one is fine because `s` lives in the coroutine frame.

But this is not:

```cpp
task<int> g(std::string_view sv) {
    co_await something();
    co_return static_cast<int>(sv.size());
}
```

If the caller passes a view into temporary or short-lived storage, the coroutine extends use across suspension. ASan catches many of these eventual accesses, especially with use-after-scope/use-after-return support enabled. ([Clang][2])

### 13.2 Destroyed promise/handle misuse

Low-level coroutine frameworks often manipulate `coroutine_handle<>` directly. Bugs in:

* double-destroy,
* resuming after destruction,
* storing handles in containers with wrong ownership rules,

tend to surface as ASan use-after-free or UBSan lifetime-adjacent failures.

### Practical guidance for coroutine-heavy code

* Be suspicious of `string_view`, `span`, raw pointers, and references that cross `co_await`.
* Treat coroutine handles as owning or non-owning with explicit rules.
* Instrument coroutine libraries themselves, not just tests.
* Prefer ASan + UBSan on coroutine code by default.

---

## 14. Custom allocators: the place where mature codebases often need the most care

Sanitizers and custom allocators are a complicated relationship.

Microsoft’s ASan docs explicitly note that the runtime reference covers intercepted functions and how to hook custom allocators. Clang ASan also relies on runtime interception and allocator metadata behavior. ([Microsoft Learn][11])

### Where problems arise

* pool allocators that bypass standard allocation interception,
* arenas that recycle freed storage aggressively,
* slab allocators with inline freelists,
* region allocators that deliberately free en masse,
* allocators embedded in containers or message queues,
* hand-rolled aligned allocators.

### What usually happens

Either:

* ASan still catches downstream misuse because accesses hit poisoned memory eventually, or
* ASan loses visibility into allocation/free history and the report becomes less precise, or
* the allocator implementation itself needs targeted exclusions or sanitizer-aware hooks.

### Best practice

Treat your allocator as a product.

It should have its own:

* unit tests under ASan,
* stress tests under TSan if shared,
* optional poison/unpoison logic if the platform/runtime APIs justify it,
* minimal, documented suppression surface.

Do not wait to discover allocator-tooling incompatibility indirectly through application-level tests.

---

## 15. Common STL and container bug patterns sanitizers expose

This is one of the most useful mental checklists for C++ engineers.

### Lifetime/view bugs

* dangling `string_view`
* dangling `span`
* storing iterators or references past container mutation
* range pipelines that outlive the source
* views over moved-from containers

### Reallocation bugs

* keeping `data()` pointer across `push_back`, `resize`, `reserve`, `shrink_to_fit`
* cached iterators after insert/erase
* pointers into `small_vector`/SSO-like implementations after growth

### Ownership bugs

* lambda captures of raw pointers from smart-owned objects
* observer pointers surviving owner destruction
* `enable_shared_from_this` misuse in partially constructed objects
* cycles that become LSan findings

### Concurrency bugs

* shared container mutation without synchronization
* publishing container contents without proper happens-before
* atomically swapping pointer owners while racing on pointee state

Sanitizers rarely say “you misused the STL.” They say “heap-use-after-free” or “data race.” The STL abstraction just makes the root cause easier to hide.

---

## 16. Feature detection macros you can actually use

Clang documents `__has_feature(address_sanitizer)`, `thread_sanitizer`, `memory_sanitizer`, and `dataflow_sanitizer`. MSVC documents `__SANITIZE_ADDRESS__`. ([Clang][10])

### Example

```cpp
#if defined(__clang__)
#  if __has_feature(address_sanitizer)
#    define MYPROJECT_ASAN 1
#  endif
#endif

#if defined(__SANITIZE_ADDRESS__)
#  define MYPROJECT_ASAN 1
#endif
```

Use this for:

* diagnostics,
* rare runtime integration hooks,
* low-level compatibility branches.

Do not use it to build a parallel codebase that behaves differently under sanitizers.

---

## 17. Windows/MSVC-specific recommendations

MSVC’s sanitizer story is centered on AddressSanitizer. Microsoft documents `/fsanitize=address`, `__SANITIZE_ADDRESS__`, `__declspec(no_sanitize_address)`, Visual Studio debugger integration, and crash-dump generation with `ASAN_SAVE_DUMPS`. ([Microsoft Learn][11])

### Practical Windows recommendations

* Add one dedicated **MSVC ASan** CI job.
* Keep PDB/debug info intact.
* Use crash dumps in CI for hard-to-repro failures.
* Do not expect parity with Clang’s full sanitizer ecosystem inside pure MSVC workflows.

If you need TSan/MSan-style depth on Windows-adjacent code, a Clang-based pipeline is often the pragmatic complement.

---

## 18. A concrete GitHub Actions-style CI layout

This is the shape I would recommend.

### Job 1: Linux Clang ASan + UBSan

* configure preset: `asan-ubsan`
* build
* run unit tests
* run representative integration subset

### Job 2: Linux Clang TSan

* configure preset: `tsan`
* build
* run race-prone tests multiple times

### Job 3: Windows MSVC ASan

* configure preset or equivalent cache vars for MSVC
* build with `/fsanitize=address`
* run unit tests
* collect dump on failure

### Job 4: Fuzz nightly

* build fuzz targets with SanitizerCoverage plus ASan/UBSan
* run with corpus and timeout budget

This aligns with documented tool overhead and intended use: ASan is around 2x slowdown, UBSan is relatively cheap, TSan is much more expensive, and SanitizerCoverage is explicitly designed as low-overhead instrumentation useful for fuzzing. ([Clang][2])

---

## 19. What to suppress and what never to suppress

Good suppression candidates:

* low-level allocator internals you have independently validated,
* context-switch or fiber glue,
* unavoidable third-party runtime shims,
* hand-written assembly bridges,
* extremely hot internal primitives with proven behavior and overwhelming cost.

Bad suppression candidates:

* “this STL test started failing”
* “TSan found a race but it never happens in production”
* “UBSan is annoying on this arithmetic path”
* “ASan only fails when we run the full suite”

The right mindset is that suppressions are a last-mile modeling tool, not a cleanup strategy.

---

## 20. A few high-value rules for modern C++ specifically

### Rule 1

Treat all non-owning vocabulary types as suspect across async boundaries:

* `string_view`
* `span`
* raw pointer
* iterator
* reference
* borrowed range/view

### Rule 2

Every container mutation invalidation rule matters more than you think.

### Rule 3

`shared_ptr` solves ownership, not synchronization.

### Rule 4

Coroutines amplify lifetime mistakes because suspension stretches time between cause and effect.

### Rule 5

Custom allocators need sanitizer strategy, not sanitizer hope.

### Rule 6

Make symbolization and presets first-class, or developers will stop using sanitized builds.

---

## 21. My recommended baseline for a serious C++20 project

If I were setting this up for a modern C++ codebase today, I would do this:

### Build system

* `CMakePresets.json` checked in
* `asan-ubsan`, `tsan`, and optional `msan` presets
* target-level sanitizer attachment via interface library

### Compiler defaults

* Clang as primary sanitizer compiler on Linux/macOS
* MSVC ASan on Windows
* `-O1 -g -fno-omit-frame-pointer` for sanitizer builds

### CI

* PR: ASan + UBSan
* nightly: TSan
* optional periodic MSan
* fuzzing for parsers/protocol code

### Coding standards

* no raw borrowed views across async/suspension unless documented
* explicit invalidation awareness in reviews
* no “benign race” language in code review without a formal memory-model argument

### Tooling

* `llvm-symbolizer` provisioned everywhere
* crash dumps enabled for Windows ASan CI
* minimal, reviewed ignorelist checked into source control

That setup is strong, realistic, and maintainable.

---

## 22. Closing thought

The biggest mistake experienced C++ engineers make with sanitizers is thinking of them as “debug aids.” In a modern codebase, they are closer to executable contracts around lifetime, bounds, initialization, and synchronization.

That matters even more in C++20 than it did in older C++, because modern abstractions let you write dangerous code that looks beautifully safe:

* a `string_view` that dangles elegantly,
* a coroutine that resumes into dead state,
* a `shared_ptr` race wrapped in clear APIs,
* a `span` into storage that reallocated two helper calls ago.

Sanitizers are how you make those contracts visible.

If you want, I can turn this into **Part III: copy-paste-ready setup for your project**, with:

* a full `CMakePresets.json`,
* a reusable `Sanitizers.cmake`,
* CI examples for GitHub Actions,
* recommended `ASAN_OPTIONS` / `UBSAN_OPTIONS` / `TSAN_OPTIONS`,
* and a curated set of tiny C++20 repro programs for each sanitizer.

[1]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html "cmake-presets(7) — CMake 4.3.1 Documentation"
[2]: https://clang.llvm.org/docs/AddressSanitizer.html "AddressSanitizer — Clang 23.0.0git documentation"
[3]: https://cmake.org/cmake/help/latest/command/target_compile_options.html "target_compile_options — CMake 4.3.1 Documentation"
[4]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html "UndefinedBehaviorSanitizer — Clang 23.0.0git documentation"
[5]: https://clang.llvm.org/docs/ThreadSanitizer.html "ThreadSanitizer — Clang 23.0.0git documentation"
[6]: https://clang.llvm.org/docs/MemorySanitizer.html "MemorySanitizer — Clang 23.0.0git documentation"
[7]: https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170&utm_source=chatgpt.com "AddressSanitizer"
[8]: https://clang.llvm.org/docs/SanitizerSpecialCaseList.html?utm_source=chatgpt.com "Sanitizer special case list - Clang - LLVM.org"
[9]: https://clang.llvm.org/docs/AddressSanitizer.html?utm_source=chatgpt.com "AddressSanitizer — Clang 23.0.0git documentation - LLVM"
[10]: https://clang.llvm.org/docs/LanguageExtensions.html?utm_source=chatgpt.com "Clang Language Extensions — Clang 23.0.0git documentation"
[11]: https://learn.microsoft.com/en-us/cpp/sanitizers/asan-building?view=msvc-170 "MSVC AddressSanitizer language, build, and debugging reference | Microsoft Learn"

---

# The Complete C/C++ Sanitizers Handbook, Part III: Copy-Paste Setup for a Modern C++ Project

This part is the implementation pack: a reusable CMake setup, presets, CI examples, runtime option sets, and small C++20 repros you can drop into a repo. It is built around current CMake preset support, target-level compile/link option wiring, Clang sanitizer docs, GCC instrumentation docs, and Microsoft’s current ASan documentation for MSVC. ([CMake][1])

## 1. The shape of the solution

For a real C++20 codebase, I would check in these files:

* `cmake/Sanitizers.cmake`
* `CMakePresets.json`
* `.github/workflows/ci.yml`
* `sanitizers/ignorelist.txt`
* `sanitizers/asan.supp`
* `sanitizers/ubsan.supp`
* `examples/sanitizers/...`

The intended usage is:

* developers run `asan-ubsan` locally,
* CI runs `asan-ubsan` on every PR,
* CI runs `tsan` separately,
* optional `msan` runs where your dependency graph allows it,
* Windows gets an MSVC ASan lane. That split matches the documented behavior and costs of the tools: ASan is around 2x slowdown, TSan is far heavier, and MSan has extra runtime/link/symbolization requirements. ([Clang][2])

## 2. `cmake/Sanitizers.cmake`

Use target-level compile and link options, not a giant blob of global flags. CMake’s docs explicitly separate `target_compile_options()` from `target_link_options()`, so sanitizer wiring should set both. ([cmake.org][3])

```cmake
# cmake/Sanitizers.cmake
include_guard(GLOBAL)

option(ENABLE_ASAN_UBSAN "Enable AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)

add_library(project_sanitizers INTERFACE)

function(project_enable_sanitizers target_name)
  if (NOT TARGET ${target_name})
    message(FATAL_ERROR "Target '${target_name}' does not exist")
  endif()

  target_link_libraries(${target_name} PRIVATE project_sanitizers)
endfunction()

if (MSVC)
  # MSVC currently centers its sanitizer support around ASan.
  if (ENABLE_ASAN_UBSAN)
    target_compile_options(project_sanitizers INTERFACE
      /fsanitize=address
      /Zi
    )

    # Microsoft recommends turning off incremental linking for ASan builds.
    target_link_options(project_sanitizers INTERFACE
      /INCREMENTAL:NO
    )
  endif()
else()
  # Common baseline for useful reports.
  target_compile_options(project_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:C,CXX>:-g>
    $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer>
  )

  if (ENABLE_ASAN_UBSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,undefined>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize-address-use-after-scope>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=address,undefined
    )
  endif()

  if (ENABLE_TSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=thread>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=thread
    )
  endif()

  if (ENABLE_MSAN)
    target_compile_options(project_sanitizers INTERFACE
      $<$<COMPILE_LANGUAGE:C,CXX>:-O1>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=memory>
      $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize-memory-track-origins=2>
    )
    target_link_options(project_sanitizers INTERFACE
      -fsanitize=memory
    )
  endif()
endif()

# Guard against incompatible combinations.
math(EXPR _san_count
  "${ENABLE_ASAN_UBSAN}+${ENABLE_TSAN}+${ENABLE_MSAN}"
)

if (_san_count GREATER 1)
  message(FATAL_ERROR
    "Choose only one sanitizer preset family at a time: "
    "ENABLE_ASAN_UBSAN, ENABLE_TSAN, or ENABLE_MSAN."
  )
endif()
```

Why these choices:

* Clang’s ASan docs recommend `-O1 -g -fno-omit-frame-pointer` and final linking with the compiler driver. ([Clang][2])
* Clang’s MSan docs likewise recommend `-O1` or higher and `-fno-omit-frame-pointer`, and explicitly say to link the final executable with `clang`, not `ld`. ([Clang][4])
* Clang’s ASan docs document `-fsanitize-address-use-after-scope`, and ASan documents optional stack-use-after-return controls. ([Clang][2])
* Microsoft documents `/fsanitize=address`, the `__SANITIZE_ADDRESS__` macro, and `__declspec(no_sanitize_address)` for selective disablement. ([Microsoft Learn][5])

## 3. Example top-level `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.23)
project(MyProject LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Sanitizers.cmake)

add_library(mycore
  src/mycore.cpp
  src/myparser.cpp
  src/myqueue.cpp
)
target_include_directories(mycore PUBLIC include)
project_enable_sanitizers(mycore)

add_executable(unit_tests
  tests/test_main.cpp
  tests/test_parser.cpp
  tests/test_queue.cpp
)
target_link_libraries(unit_tests PRIVATE mycore)
project_enable_sanitizers(unit_tests)

add_test(NAME unit_tests COMMAND unit_tests)

add_executable(example_string_view examples/sanitizers/asan_string_view.cpp)
project_enable_sanitizers(example_string_view)

add_executable(example_tsan_queue examples/sanitizers/tsan_race.cpp)
project_enable_sanitizers(example_tsan_queue)

add_executable(example_ubsan_shift examples/sanitizers/ubsan_shift.cpp)
project_enable_sanitizers(example_ubsan_shift)
```

This keeps sanitizer attachment explicit and target-scoped, which is the right default for medium-to-large C++ repos. ([cmake.org][3])

## 4. `CMakePresets.json`

CMake presets were added in CMake 3.19, and the official docs define `CMakePresets.json` as the shared project-wide file and `CMakeUserPresets.json` as the local-user file that should not be checked in. ([CMake][1])

```json
{
  "version": 10,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 23,
    "patch": 0
  },
  "$comment": "Shared build personalities for normal, ASan+UBSan, TSan, and MSan builds.",
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "BUILD_TESTING": "ON"
      }
    },
    {
      "name": "debug",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "asan-ubsan",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN_UBSAN": "ON",
        "ENABLE_TSAN": "OFF",
        "ENABLE_MSAN": "OFF"
      },
      "environment": {
        "ASAN_OPTIONS": "check_initialization_order=1:detect_leaks=1",
        "UBSAN_OPTIONS": "print_stacktrace=1:log_path=stderr"
      }
    },
    {
      "name": "tsan",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN_UBSAN": "OFF",
        "ENABLE_TSAN": "ON",
        "ENABLE_MSAN": "OFF"
      }
    },
    {
      "name": "msan",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN_UBSAN": "OFF",
        "ENABLE_TSAN": "OFF",
        "ENABLE_MSAN": "ON"
      }
    },
    {
      "name": "windows-msvc-asan",
      "inherits": "base",
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN_UBSAN": "ON",
        "ENABLE_TSAN": "OFF",
        "ENABLE_MSAN": "OFF"
      },
      "condition": {
        "type": "equals",
        "lhs": "${hostSystemName}",
        "rhs": "Windows"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "debug",
      "configurePreset": "debug"
    },
    {
      "name": "asan-ubsan",
      "configurePreset": "asan-ubsan"
    },
    {
      "name": "tsan",
      "configurePreset": "tsan"
    },
    {
      "name": "msan",
      "configurePreset": "msan"
    },
    {
      "name": "windows-msvc-asan",
      "configurePreset": "windows-msvc-asan"
    }
  ],
  "testPresets": [
    {
      "name": "asan-ubsan",
      "configurePreset": "asan-ubsan",
      "output": {
        "outputOnFailure": true
      },
      "execution": {
        "stopOnFailure": true
      }
    },
    {
      "name": "tsan",
      "configurePreset": "tsan",
      "output": {
        "outputOnFailure": true
      }
    },
    {
      "name": "msan",
      "configurePreset": "msan",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```

A few notes on those environment choices:

* Clang documents `ASAN_OPTIONS=check_initialization_order=1`, but also notes it is not supported on macOS. ([Clang][2])
* Clang documents `ASAN_OPTIONS=detect_stack_use_after_return=0` to disable UAR detection when runtime mode is used, which is useful if you need a compatibility escape hatch. ([Clang][2])
* Clang documents `UBSAN_OPTIONS=log_path=...` for redirecting diagnostics. ([Clang][6])

## 5. Commands developers actually run

```bash
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan
```

```bash
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan
```

```bash
cmake --preset msan
cmake --build --preset msan
ctest --preset msan
```

That is the entire point of presets: shared, reproducible build personalities for devs and CI. ([CMake][1])

## 6. Recommended runtime environment settings

These are the option sets I would start with, not because they are the only valid ones, but because they are stable and useful.

### 6.1 ASan

```bash
export ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer)"
export ASAN_OPTIONS="check_initialization_order=1:detect_leaks=1"
```

Good reasons:

* Clang documents online/offline symbolization and `ASAN_OPTIONS=symbolize=0` for offline processing. ([Clang][2])
* Clang documents `check_initialization_order=1`. ([Clang][2])
* Clang documents leak detection defaults on Linux and `detect_leaks=1` on macOS. ([Clang][2])

If you hit external-library noise, Clang documents ASan suppressions via:

```bash
export ASAN_OPTIONS="suppressions=/absolute/path/to/sanitizers/asan.supp"
```

and the suppression format uses `interceptor_via_fun:` and `interceptor_via_lib:` entries. ([Clang][2])

### 6.2 UBSan

```bash
export UBSAN_OPTIONS="print_stacktrace=1:log_path=stderr"
```

If you are fuzzing integer-heavy code and want signal without log explosions, Clang documents `UBSAN_OPTIONS=silence_unsigned_overflow=1` together with recovery mode for unsigned-overflow checks. ([Clang][6])

### 6.3 TSan

I would usually start with no custom `TSAN_OPTIONS` at all. TSan is already heavy and high-signal. If you need more aggressive scheduling perturbation during a targeted repro, Clang documents adaptive delay options such as:

```bash
export TSAN_OPTIONS="enable_adaptive_delay=1:adaptive_delay_aggressiveness=50"
```

or more aggressive values for stress scenarios. ([Clang][7])

### 6.4 MSan

```bash
export MSAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer)"
```

Clang explicitly documents that MSan uses an external symbolizer and that `llvm-symbolizer` should be in `PATH` or pointed to via `MSAN_SYMBOLIZER_PATH`. ([Clang][4])

## 7. `sanitizers/ignorelist.txt`

Use an ignorelist for low-level code that sanitizers do not model well, not for ordinary bugs. Clang documents sanitizer special case lists across the sanitizer family. ([Clang][2])

```text
# sanitizers/ignorelist.txt

# Third-party or tool-hostile source file
src:third_party/legacy_crc.cpp

# A known low-level hot function
fun:myproject::detail::context_switch

# Example of narrowing to a specific type/global when needed
type:myproject::WireHeader
global:legacy_buffer
```

In CMake, wire it like this for Clang/GCC builds that use ignorelists:

```cmake
if (NOT MSVC)
  target_compile_options(project_sanitizers INTERFACE
    $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize-ignorelist=${CMAKE_SOURCE_DIR}/sanitizers/ignorelist.txt>
  )
endif()
```

I would keep this file very small and review every line like production code.

## 8. `sanitizers/asan.supp`

This is for external-library runtime suppressions only, not your own code. Clang documents runtime suppression entries like `interceptor_via_fun:` and `interceptor_via_lib:`. ([Clang][2])

```text
# sanitizers/asan.supp
interceptor_via_lib:liblegacy_plugin.so
interceptor_via_fun:legacy_c_api_entrypoint
```

## 9. `sanitizers/ubsan.supp`

Clang documents UBSan runtime suppressions through `UBSAN_OPTIONS=suppressions=...`. ([Clang][6])

```text
# sanitizers/ubsan.supp
alignment:third_party/packed_structs.cpp
signed-integer-overflow:legacy_numeric_path.cpp
```

Use these only when you truly have a compatibility reason and a plan to remove them.

## 10. Optional source-level macros for selective disablement

For Clang/GCC and MSVC, these are the macros I would actually keep around.

```cpp
// include/myproject/sanitize_macros.hpp
#pragma once

#if defined(_MSC_VER) && defined(__SANITIZE_ADDRESS__)
  #define MYPROJECT_NO_ASAN __declspec(no_sanitize_address)
#elif defined(__clang__) || defined(__GNUC__)
  #define MYPROJECT_NO_ASAN __attribute__((no_sanitize_address))
#else
  #define MYPROJECT_NO_ASAN
#endif

#if defined(__clang__)
  #if __has_feature(thread_sanitizer)
    #define MYPROJECT_TSAN_ENABLED 1
  #endif
  #if __has_feature(address_sanitizer)
    #define MYPROJECT_ASAN_ENABLED 1
  #endif
  #if __has_feature(memory_sanitizer)
    #define MYPROJECT_MSAN_ENABLED 1
  #endif
#endif

#if defined(__SANITIZE_ADDRESS__)
  #define MYPROJECT_ASAN_ENABLED 1
#endif
```

This is grounded in Clang’s documented `__has_feature(...)` support and Microsoft’s documented `__SANITIZE_ADDRESS__` and `__declspec(no_sanitize_address)`. ([Microsoft Learn][5])

## 11. GitHub Actions CI example

This is the shape I recommend: one fast checked lane, one separate race lane, and one Windows ASan lane.

```yaml
name: ci

on:
  push:
  pull_request:

jobs:
  linux-asan-ubsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build clang lld llvm cmake

      - name: Configure
        run: cmake --preset asan-ubsan

      - name: Build
        run: cmake --build --preset asan-ubsan --parallel

      - name: Test
        env:
          ASAN_SYMBOLIZER_PATH: /usr/bin/llvm-symbolizer
          ASAN_OPTIONS: check_initialization_order=1:detect_leaks=1
          UBSAN_OPTIONS: print_stacktrace=1:log_path=stderr
        run: ctest --preset asan-ubsan

  linux-tsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build clang lld llvm cmake

      - name: Configure
        run: cmake --preset tsan

      - name: Build
        run: cmake --build --preset tsan --parallel

      - name: Test
        run: ctest --preset tsan

  windows-msvc-asan:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4

      - name: Configure
        run: cmake --preset windows-msvc-asan

      - name: Build
        run: cmake --build --preset windows-msvc-asan --parallel

      - name: Test
        shell: cmd
        run: |
          set ASAN_SAVE_DUMPS=asan_failures.dmp
          ctest --preset windows-msvc-asan --output-on-failure
```

Why this split:

* Clang documents TSan’s major runtime and memory overhead, making it a separate lane by design. ([Clang][7])
* Microsoft documents `ASAN_SAVE_DUMPS=...` for post-mortem debugging in distributed or cloud workflows. ([Microsoft Learn][8])
* CMake presets are designed precisely for shared CI/developer configurations. ([CMake][1])

## 12. Tiny C++20 repros

These are deliberately small and realistic.

### 12.1 ASan: dangling `std::string_view`

```cpp
// examples/sanitizers/asan_string_view.cpp
#include <iostream>
#include <string>
#include <string_view>

std::string_view make_prefix() {
    std::string s = "abcdef";
    return std::string_view{s.data(), 3};
}

int main() {
    auto v = make_prefix();
    std::cout << v << '\n'; // likely ASan use-after-scope / UAR style failure
}
```

This is exactly the kind of lifetime bug modern C++ makes easy to write cleanly.

### 12.2 ASan: iterator invalidation

```cpp
// examples/sanitizers/asan_vector_iter.cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3};
    auto it = v.begin();
    v.push_back(4); // may reallocate
    std::cout << *it << '\n'; // stale iterator
}
```

### 12.3 UBSan: signed overflow

```cpp
// examples/sanitizers/ubsan_overflow.cpp
#include <iostream>
#include <limits>

int main() {
    int x = std::numeric_limits<int>::max();
    std::cout << (x + 1) << '\n';
}
```

Clang’s UBSan docs use this exact bug family as a canonical example and document the resulting runtime error report. ([Clang][6])

### 12.4 UBSan: invalid shift

```cpp
// examples/sanitizers/ubsan_shift.cpp
#include <cstdint>
#include <iostream>

int main() {
    std::uint32_t bits = 32;
    std::uint32_t mask = 1u << bits;
    std::cout << mask << '\n';
}
```

### 12.5 TSan: unsynchronized flag

```cpp
// examples/sanitizers/tsan_flag.cpp
#include <thread>

bool done = false;

int main() {
    std::thread t([] {
        while (!done) {
        }
    });

    done = true;
    t.join();
}
```

TSan is specifically a data-race detector. ([Clang][7])

### 12.6 TSan: `shared_ptr` does not synchronize pointee access

```cpp
// examples/sanitizers/tsan_shared_ptr.cpp
#include <memory>
#include <thread>

struct Counter {
    int value = 0;
};

int main() {
    auto c = std::make_shared<Counter>();

    std::thread t1([c] { ++c->value; });
    std::thread t2([c] { ++c->value; });

    t1.join();
    t2.join();
}
```

### 12.7 MSan: uninitialized branch

```cpp
// examples/sanitizers/msan_uninit_branch.cpp
#include <iostream>

int main() {
    int x;
    if (x) {
        std::cout << "initialized?\n";
    }
}
```

Clang’s MSan docs explicitly list uninitialized values used in conditional branches as a reportable case. ([Clang][4])

## 13. Optional fuzz target example

SanitizerCoverage is particularly useful with fuzzing. Clang documents SanitizerCoverage as instrumentation for tracing PCs, edge coverage, and more. Microsoft also documents `/fsanitize=fuzzer` and recommends combining it with `/fsanitize=address`; it automatically enables sanitizer coverage options such as edge instrumentation and inline 8-bit counters. ([Clang][9])

```cpp
// examples/sanitizers/fuzz_parse.cpp
#include <cstddef>
#include <cstdint>
#include <string_view>

extern int parse_message(std::string_view);

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    std::string_view sv(reinterpret_cast<const char*>(data), size);
    (void)parse_message(sv);
    return 0;
}
```

For Clang-based builds, the usual pattern is to combine fuzzing with ASan and UBSan so malformed inputs turn into immediate actionable failures. Microsoft documents the same recommendation for `/fsanitize=fuzzer` plus `/fsanitize=address`. ([Microsoft Learn][5])

## 14. Recommended build commands for the examples

### Clang or GCC, ASan + UBSan

```bash
clang++ -std=c++20 -O1 -g -fno-omit-frame-pointer \
  -fsanitize=address,undefined \
  examples/sanitizers/asan_string_view.cpp -o asan_string_view
```

Clang documents this style of command directly for ASan. ([Clang][2])

### Clang, TSan

```bash
clang++ -std=c++20 -O1 -g -fno-omit-frame-pointer \
  -fsanitize=thread \
  examples/sanitizers/tsan_flag.cpp -o tsan_flag
```

### Clang, MSan

```bash
clang++ -std=c++20 -O1 -g -fno-omit-frame-pointer \
  -fsanitize=memory -fsanitize-memory-track-origins=2 \
  examples/sanitizers/msan_uninit_branch.cpp -o msan_uninit_branch
```

Clang’s MSan docs document the need to link the final executable with the compiler driver and the usefulness of origin tracking and symbolization. ([Clang][4])

### MSVC, ASan

```bat
cl /std:c++20 /fsanitize=address /Zi examples\sanitizers\asan_vector_iter.cpp
```

Microsoft documents `/fsanitize=address` and related runtime/debugging integration. ([Microsoft Learn][8])

## 15. My default policy choices

This is the policy I would adopt for a professional C++ codebase.

Use **ASan + UBSan** as the default checked build because it provides broad coverage at acceptable cost and catches the majority of day-to-day native-code correctness bugs. Clang’s docs make ASan cheap enough for frequent use and UBSan selective enough to tune. ([Clang][2])

Use **TSan** as a separate CI lane because its overhead is much larger and it solves a different class of problems: data races. ([Clang][7])

Use **MSan** only when you can rebuild enough of the code and dependencies to make it valuable. Clang explicitly notes MSan runtime/link constraints and its external symbolizer requirement. ([Clang][4])

Keep **ignorelists and suppressions tiny**. Clang’s ASan docs are blunt that ASan is not expected to produce false positives, and that is the right mindset to preserve. ([Clang][2])

## 16. The shortest copy-paste starter pack

If you want the minimum version of everything above, use this:

* `Sanitizers.cmake` with `ENABLE_ASAN_UBSAN`, `ENABLE_TSAN`, `ENABLE_MSAN`
* `CMakePresets.json` with `asan-ubsan`, `tsan`, `msan`
* CI with:

  * Linux Clang `asan-ubsan`
  * Linux Clang `tsan`
  * Windows MSVC ASan
* env:

  * `ASAN_SYMBOLIZER_PATH=$(command -v llvm-symbolizer)`
  * `ASAN_OPTIONS=check_initialization_order=1:detect_leaks=1`
  * `UBSAN_OPTIONS=print_stacktrace=1:log_path=stderr`
  * `MSAN_SYMBOLIZER_PATH=$(command -v llvm-symbolizer)`

Every one of those choices maps directly to current CMake and sanitizer documentation. ([CMake][1])

I can turn this next into a **single repository skeleton** with all these files laid out exactly as a starter project, including a ready-to-build directory tree.

[1]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html "cmake-presets(7) — CMake 4.3.1 Documentation"
[2]: https://clang.llvm.org/docs/AddressSanitizer.html "AddressSanitizer — Clang 23.0.0git documentation"
[3]: https://cmake.org/cmake/help/latest/command/target_compile_options.html "target_compile_options — CMake 4.3.1 Documentation"
[4]: https://clang.llvm.org/docs/MemorySanitizer.html "MemorySanitizer — Clang 23.0.0git documentation"
[5]: https://learn.microsoft.com/en-us/cpp/sanitizers/asan-building?view=msvc-170 "MSVC AddressSanitizer language, build, and debugging reference | Microsoft Learn"
[6]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html "UndefinedBehaviorSanitizer — Clang 23.0.0git documentation"
[7]: https://clang.llvm.org/docs/ThreadSanitizer.html "ThreadSanitizer — Clang 23.0.0git documentation"
[8]: https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170 "AddressSanitizer | Microsoft Learn"
[9]: https://clang.llvm.org/docs/SanitizerCoverage.html "SanitizerCoverage — Clang 23.0.0git documentation"