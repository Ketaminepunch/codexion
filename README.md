_This project has been created as part of the 42 curriculum by vsack._

# Codexion

## Description

Codexion is 42's classic Dining Philosophers problem, reskinned as a
co-working space for coders and extended with configurable scheduling
policies, a mandatory dongle cooldown, and a hand-rolled priority queue (no
standard library heap allowed).

`number_of_coders` coders sit in a circle around a shared Quantum Compiler.
Between each pair of neighboring coders sits one USB dongle, so there are
always exactly as many dongles as coders. To compile, a coder needs both
their left and right dongle at the same time. Each coder cycles through four
states — taking dongles, compiling, debugging, refactoring — then loops back
to taking dongles again. If a coder goes too long without starting a new
compile, they burn out and the whole simulation stops immediately. The
simulation also stops successfully once every coder has completed
`number_of_compiles_required` compiles.

Two scheduling policies decide who wins a contested dongle:

- `fifo` — First In, First Out: whoever requested the dongle first gets it.
- `edf` — Earliest Deadline First: whoever is closest to burning out
  (`last_compile_start + time_to_burnout`) gets it, with arrival time as a
  tie-breaker.

Every dongle also enforces a mandatory cooldown: once released, it can't be
picked up again until `dongle_cooldown` milliseconds have passed.

## Instructions

Build with:

    make

This produces the `codexion` binary at the repo root. `make clean` removes
object files, `make fclean` also removes the binary, `make re` rebuilds from
scratch.

Run it with all 8 mandatory arguments, in order:

    ./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler

- `number_of_coders` — how many coders (and dongles).
- `time_to_burnout` (ms) — how long a coder can go without starting a
  compile before burning out.
- `time_to_compile` (ms) — how long a compile takes.
- `time_to_debug` (ms) — how long debugging takes.
- `time_to_refactor` (ms) — how long refactoring takes.
- `number_of_compiles_required` — the simulation succeeds once every coder
  reaches this many compiles.
- `dongle_cooldown` (ms) — how long a dongle stays unavailable after being
  released.
- `scheduler` — exactly `fifo` or `edf`.

Example:

    ./codexion 5 800 50 20 20 8 400 edf

Every state change is logged as `timestamp_in_ms coder_id message`, e.g.:

    0 1 has taken a dongle
    2 1 has taken a dongle
    2 1 is compiling
    202 1 is debugging
    402 1 is refactoring

## Blocking cases handled

**Deadlock prevention (Coffman's conditions):** a coder always acquires the
lower-indexed dongle first and the higher-indexed one second
(`coder_take_dongles`/`acquire_pair` in `src/coders.c`/`src/utils.c`), which
breaks the circular-wait condition — the standard dining philosophers fix.
The single-coder edge case (`left == right`) is special-cased to acquire one
dongle instead of trying to take the same dongle twice.

**Starvation prevention:** each dongle keeps its own priority queue (heap) of
pending requests (`src/scheduling.c`); a coder only proceeds once it's at the
front of that queue, ordered by `fifo` or `edf`. Under `edf`, the coder
closest to burning out is always served next, which is what guarantees
liveness.

**Stop propagation while waiting:** a coder blocked in `dongle_acquire`
re-checks `stop_requested(sim)` as part of its wait condition
(`src/dongle.c`), so once the monitor thread calls `broadcast_stop`, every
coder currently waiting on a dongle wakes up and exits instead of finishing
one more full cycle first.

**Cooldown handling:** `dongle_release` stamps
`ready_at_ms = now + dongle_cooldown` and flips the dongle to `COOLING`;
`dongle_acquire`'s wait condition checks both `state == HELD` and
`now < ready_at_ms`, so a coder can't grab a dongle mid-cooldown even if it's
otherwise free.

**Burnout precision:** the monitor thread (`monitor_thread`,
`src/monitoring.c`) polls every coder's `last_compile_start` in a tight loop
(`usleep(1000)` between checks), keeping burnout detection within the
subject's 10ms tolerance.

**Log serialization:** every log line goes through `log_action`
(`src/coders.c`), which holds `sim->out_lock` for the entire `printf` call,
so two threads can never interleave a line.


## Thread synchronization mechanisms

- `pthread_mutex_t` per dongle (`t_dongle.lock`) protects its `state`,
  `ready_at_ms`, and its request heap.
- `pthread_cond_t` per dongle (`t_dongle.condition`) is what coders block on
  while waiting their turn; both `dongle_release` and `broadcast_stop`
  broadcast on it so every waiter re-checks its wait condition instead of a
  single arbitrary one being woken.
- `pthread_mutex_t` per coder (`t_coder.lock`) protects `last_compile_start`
  and `compiles_finished`, since both the coder's own thread and the monitor
  thread read/write them concurrently.
- `sim->stop_lock` protects the single `stop_flag`, read by every coder
  (`coder_should_stop`) and written once by the monitor (`broadcast_stop`).
- `sim->out_lock` serializes all `printf` calls through `log_action`.

Race conditions are prevented by never touching shared state (a dongle's
state/heap, a coder's timing fields, the stop flag, stdout) without holding
the matching lock first, and by broadcasting rather than signaling on a
dongle's condition variable, so a coder whose wait condition no longer holds
doesn't stay asleep waiting for its own wakeup that never comes.

## Resources

- [POSIX Threads Programming (LLNL tutorial)](https://hpc-tutorials.llnl.gov/posix/) —
  general introduction to `pthread_create`/`join`, mutexes, and condition
  variables.
- [`man pthread_mutex_lock`](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html),
  [`man pthread_cond_wait`](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html) —
  POSIX reference for the exact locking/waiting semantics used throughout
  `src/dongle.c`.
- [Dining Philosophers problem (Wikipedia)](https://en.wikipedia.org/wiki/Dining_philosophers_problem) —
  background on the classic problem this subject reskins, including the
  resource-hierarchy deadlock-avoidance strategy used here.
- [Earliest Deadline First scheduling (Wikipedia)](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling) —
  background on the `edf` scheduling policy.
- [The Little Book of Semaphores (Downey)](https://greenteapress.com/wp/semaphores/) —
  broader reference on classic synchronization problems and patterns.

TODO: list any project-specific documentation, articles, or tutorials
referenced while building this project.

### AI usage

Claude (Anthropic) was used during this project for:

- Diagnosing and fixing four memory leaks in `sim_init`/`main` (unfreed
  `dongle_arr`, `coder_arr`, per-dongle `heap.items`, and `thread_args`),
  including making `sim_init` clean up after itself on partial failure and
  restructuring `main`'s error handling to stay within the norm's 25-line
  and 5-function-per-file limits.
- Verifying the leak fixes with `valgrind`, including a fault-injection test
  that forced `array_slot_init` to fail partway through its loop to confirm
  the partial-failure cleanup frees exactly the right slots and nothing
  else.
- Helping understanding concepts like mutex and multithreading as a whole.

