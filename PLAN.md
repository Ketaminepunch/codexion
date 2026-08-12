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
  - Builds clean (`make re`).
- n == 1 self-deadlock fixed: `coder_take_dongles` (`srcs/utils.c`) is now a
  proper `if (left == right) {...} else if (left < right) {...} else {...}`
  chain (was two independent `if`s, so the `left == right` case fell through
  into the `else` and double-acquired the same dongle anyway). Verified: `./
  codexion 1 ... ` runs 3 compile cycles clean, no hang.
- Coder ids are 1-indexed (`sim->coder_arr[i].id = i + 1` in
  `array_slot_init`, `srcs/sim.c`), matching the subject ("number ranging
  from 1 to number_of_coders") and the example log format. `id` is only ever
  compared for equality or printed (`srcs/dongle.c`, `srcs/monitoring.c`,
  `srcs/utils.c`), never used as an array index, so the +1 was a
  single-line, safe change.
- Monitor thread (`srcs/monitoring.c`): `check_burnout`, `check_success`,
  `broadcast_stop`, `monitor_thread`. Wired into `main` via
  `create_and_join` (`srcs/sim.c`): spawns coders, spawns the monitor, joins
  the monitor first (it only returns once `stop_flag` is set and every
  dongle's condvar has been broadcast), then joins the coders. Verified:
  burnout logged within ~1ms of the threshold (e.g. `101 1 burned out` for
  `time_to_burnout=100`), program exits instead of hanging.
- `log_action` serialized via `out_lock`, format matches the subject exactly.

## Remaining work

### Known issue: stop signal not checked while waiting for a dongle
Found 2026-08-11 while comparing timing against classmates' implementations
on `./codexion 5 800 50 20 20 8 400 edf`: theirs finishes in ~800ms / 21 log
lines (burns out right at the deadline and stops), mine takes ~2300-2800ms /
36-41 lines. Confirmed via log inspection: a coder that already burned out
(`801 1 burned out`) goes on to acquire dongles and run a whole extra
compile cycle (`2250 1 has taken a dongle` ...) over a second later.

Root cause: `coder_should_stop` (`srcs/utils.c`) is only checked at the top
of `coder_thread`'s while loop. `dongle_acquire`'s wait loop
(`srcs/dongle.c:55-57`) has no idea the simulation ended — it just keeps
blocking in `dongle_wait_turn` until it legitimately wins the dongle, no
matter how long that takes after `stop_flag` was set.

Fix plan (not yet implemented):
- `dongle_acquire(t_dongle *dongle, t_coder *coder, t_args *args)` needs
  `sim` added to its signature so it can read `stop_flag`, and its return
  type needs to change from `void` to `int` (1 = acquired, 0 = aborted due
  to stop).
- Wait loop becomes `while (!stop_requested(sim) && (state == HELD || ...))`
  — ANDing in the stop check so it exits the moment stop fires, not just
  when eligible.
- After the loop, re-check eligibility directly (don't just branch on
  `stop_requested`) — the loop can exit either because it's eligible or
  because stop fired, and only re-checking the original three-part
  condition tells those apart.
- On abort: the coder already pushed its own request into the heap before
  the loop (line 54) — can't just `heap_pop` (that removes index 0, which
  might be the *other* coder's request). Since heap capacity is always <= 2
  (ring topology guarantees at most 2 waiters per dongle), if we're not at
  index 0 we must be at index 1, so removal is simple — but needs to handle
  both cases (we're at 0 alone/top, or we're at 1).
- `coder_take_dongles` (`srcs/utils.c`) becomes `int`-returning too: if the
  first `dongle_acquire` aborts, return 0 (nothing held, nothing to clean
  up). If the first succeeds but the second aborts, must `dongle_release`
  the first before returning 0 (otherwise it stays HELD forever).
- `coder_thread`'s loop (`srcs/utils.c:82`) needs
  `if (!coder_take_dongles(coder, sim)) break;` instead of an unchecked call,
  so an aborted acquisition exits the thread immediately instead of
  continuing into `"is compiling"`.
- Once fixed, re-run the FIFO-vs-EDF starvation comparison — the stats
  gathered before this fix are unreliable (a stuck/slow-stopping coder can
  keep the whole program running long past when the scheduler decision
  actually mattered).

### Testing
- [x] 1-coder edge case.
- [x] Burnout log timing within the 10ms tolerance.
- [ ] fifo vs edf under contention, once the stop-propagation bug above is
  fixed (heap arbitration itself was verified correct in isolation — always
  picks the lowest deadline/earliest arrival when real contention happens —
  but real contention turned out to be rare with the first parameter set
  tried; needs cooldown large relative to compile/debug/refactor to show
  up reliably).
- [ ] valgrind for leaks, helgrind/tsan for data races if available.

### README.md
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
