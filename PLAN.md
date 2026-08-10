Working plan for Codexion (42's Dining Philosophers, reskinned + extended with
scheduler policies, dongle cooldown, and a mandatory hand-rolled priority queue).
Not a submission file — just a personal checklist. Delete before turning in if
that matters for the eval, but it's untracked either way unless you `git add` it.

## Done so far

- Project skeleton: `Makefile` (NAME/all/clean/fclean/re, `-Wall -Wextra -Werror
  -pthread`, explicit `SRCS` list — no `$(wildcard ...)`), `includes/`, `srcs/`.
- Argument parsing (`srcs/parsing.c`, `srcs/main.c`, `includes/codexion.h`):
  - `is_valid_number` — rejects NULL/empty/non-digit strings (no `-`/`+`
    allowed, so negatives are rejected implicitly).
  - `set_number` — validates + `strtoull`s one numeric arg into a `uint64_t*`.
  - `parse_scheduler` — exact-match `strcmp` against `"fifo"`/`"edf"`, stores
    into a `t_scheduler` enum (`FIFO`/`EDF`).
  - `parse_args` — calls the above for `av[1]`..`av[7]` (the 7 numeric args:
    num_coders, time_to_burnout, time_to_compile, time_to_debug,
    time_to_refactor, compiles_required, dongle_cooldown) then `av[8]`
    (scheduler).
  - `main` checks `argc == 9` before calling `parse_args`.
- Builds clean (`make re`), smoke-tested valid/invalid scheduler args.
- Core data structures (`includes/codexion.h`): `t_request`, `t_heap`,
  `t_dongle`, `t_coder`, `t_args`, `t_simulation_state`, `t_thread_arg`.
  Deadlock avoidance: resource hierarchy — always acquire the lower-indexed
  dongle first. Dongle `i` sits between coder `i` and coder `(i+1)%n`; n == 1
  collapses to a single dongle.
- Dongle acquire/release with cooldown (`srcs/dongle.c`): `get_time_ms`,
  `dongle_release`, `dongle_wait_turn`, `dongle_acquire`. Condition-variable
  wait combines "held" and "cooling" cases, plus a third heap-top check (see
  next item) so the correct coder wakes first.
- Hand-rolled binary heap (`srcs/heap.c`): `compare_requests` (fifo by arrival
  time, edf by deadline with arrival-time tie-breaker), `heap_swap`,
  `heap_push`, `heap_pop`, `most_urgent_child`. Fully array-backed, sift-up/
  sift-down from scratch.
- Coder thread lifecycle (`srcs/utils.c`): `log_action` (mutex-serialized,
  `timestamp_in_ms id message` format via `PRIu64`), `coder_should_stop`,
  `coder_take_dongles`, `coder_release_dongle`, `coder_thread` (the full
  acquire -> compile -> release -> debug -> refactor loop, updating
  `last_compile_start`/`compiles_finished` under `coder->lock`).
- Wired coder threads up in `main` (`srcs/main.c`, `srcs/sim.c`):
  - `sim_init`/`array_slot_init` (`srcs/sim.c`) — allocate `dongle_arr`/
    `coder_arr`, init `out_lock`/`stop_lock`/`start_time`/`stop_flag`, and
    per-slot init each dongle (state/lock/condition/heap, heap capacity 2 —
    max simultaneous waiters per dongle in the circular topology) and each
    coder (id, left/right via `(i - 1 + n) % n`, lock, `last_compile_start`
    seeded to `get_time_ms()` so the future monitor thread doesn't see a
    false burnout at t=0).
  - `spawn_coders`/`join_coders` (`srcs/main.c`) — `malloc`'d `t_thread_arg`
    array sized to `num_coders`, `pthread_create` per coder storing the id
    into `coder->ticket`, then `pthread_join` all of them.
  - Builds clean (`make re`); not yet smoke-tested end-to-end (no monitor
    thread yet, and the `n == 1` double-acquire edge case in
    `coder_take_dongles` — see below — isn't handled).

## Remaining work

### Known issue: n == 1 self-deadlock
When `num_coders == 1`, `left == right == 0` (same dongle both sides).
`coder_take_dongles` as written will call `dongle_acquire` on that same
dongle twice in a row — the second call pushes the coder into the heap again
and waits for `state != HELD`, but the coder itself is the one holding it.
Permanent self-deadlock. Needs a `left == right` special case (skip the
second acquire/release) before the 1-coder test in step 9 will work.

### 7. Monitor thread
- Separate thread (not one of the coder threads).
- Polls/detects when `now - last_compile_start > time_to_burnout` for any
  coder.
- Must log the burnout within 10ms of the real event, then signal all coder
  threads to stop and `pthread_join` them.
- Also responsible for detecting the "everyone hit compiles_required" success
  stop condition.
- Needs to `pthread_cond_broadcast` every dongle's condition variable when it
  sets the stop flag, so coder threads blocked in `dongle_wait_turn` actually
  wake up and notice the stop rather than hanging.

### 8. Serialized logging
- `log_action` (in `srcs/utils.c`) is already written and mutex-protected —
  just needs `out_lock` to actually get `pthread_mutex_init`'d during setup
  (see 6b).
- Exact format: `timestamp_in_ms X has taken a dongle / is compiling / is
  debugging / is refactoring / burned out`.
- Timestamp = ms since simulation start (`gettimeofday` is fine per the
  subject, `clock_gettime` also allowed).

### 9. Testing
- 1-coder edge case (only one dongle should exist on the table at all).
- fifo vs edf under contention (many coders, tight `time_to_burnout`).
- Confirm edf actually prevents starvation where fifo might not.
- valgrind for leaks, helgrind/tsan for data races if available.
- Confirm burnout log timing stays within the 10ms tolerance.

### 10. README.md
Required sections (see subject ch. VII): italic first line with 42 logins,
Description, Instructions, Resources (incl. how AI was used and for what),
plus project-specific required sections: **Blocking cases handled** (deadlock
prevention/Coffman's conditions, starvation prevention, cooldown handling,
burnout precision, log serialization) and **Thread synchronization
mechanisms** (mutexes/cond vars used, how race conditions are prevented).
Written in English.

## Reminders from the subject
- Allowed external functions only: pthread_create/join,
  pthread_mutex_init/lock/unlock/destroy, pthread_cond_init/wait/
  timedwait/signal/broadcast/destroy, gettimeofday, clock_gettime, usleep,
  write, malloc, free, printf, fprintf, strcmp, strlen, atoi, memset. No
  `exit()` — propagate failure via return codes instead.
- Norm: max 25 lines/function, max 5 functions/file, tabs, no `for` loops.
- No crashes/leaks tolerated — any segfault/double-free/etc. is an automatic
  0 during eval.
