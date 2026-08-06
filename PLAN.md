Working plan for Codexion (42's Dining Philosophers, reskinned + extended with
scheduler policies, dongle cooldown, and a mandatory hand-rolled priority queue).
Not a submission file — just a personal checklist. Delete before turning in if
that matters for the eval, but it's untracked either way unless you `git add` it.

## Done so far

- Project skeleton: `Makefile` (NAME/all/clean/fclean/re, `-Wall -Wextra -Werror
  -pthread`, auto-picks up every `srcs/*.c` via `$(wildcard ...)`), `includes/`,
  `srcs/`.
- Argument parsing (`srcs/parsing.c`, `srcs/main.c`, `includes/codexion.h`):
  - `is_valid_number` — rejects NULL/empty/non-digit strings (no `-`/`+`
    allowed, so negatives are rejected implicitly).
  - `set_number` — validates + `atoi`s one numeric arg into a `long*`.
  - `parse_scheduler` — exact-match `strcmp` against `"fifo"`/`"edf"`, stores
    into a `t_scheduler` enum (`FIFO`/`EDF`).
  - `parse_args` — calls the above for `av[1]`..`av[7]` (the 7 numeric args:
    num_coders, time_to_burnout, time_to_compile, time_to_debug,
    time_to_refactor, compiles_required, dongle_cooldown) then `av[8]`
    (scheduler).
  - `main` checks `argc == 9` before calling `parse_args`.
- Builds clean (`make re`), smoke-tested valid/invalid scheduler args.

## Remaining work

### 3. Core data structures
- `t_dongle`: mutex, state (free / held / cooling down), cooldown-ready
  timestamp, condition variable, maybe a wait-queue pointer.
- `t_coder`: id (1..num_coders), state, last_compile_start timestamp,
  compiles_done counter, pointers to left/right `t_dongle`.
- Shared sim state struct: the parsed `t_args`, array/list of coders, array of
  dongles, stop flag, log mutex, start timestamp, the scheduler's heap.
- Scheduler node struct: whatever the heap stores per waiting request (coder
  id, arrival time for fifo, deadline = last_compile_start + time_to_burnout
  for edf).
- Pick and write down (in the README later) which deadlock-avoidance strategy
  breaks the circular-wait condition — e.g. asymmetric pickup order, or a
  central arbitrator only granting a pair when both dongles are free.

### 4. Dongle acquire/release with cooldown
- Mutex-protected state machine: free -> held -> cooling -> free.
- After release, dongle must stay unavailable until `dongle_cooldown` ms have
  passed — track a "ready again at" timestamp, checked before granting.
- Use condition variables to sleep waiting coders instead of busy-polling.

### 5. Priority queue / heap for scheduler
- Hand-rolled binary heap (no library priority queue allowed).
- fifo: order by request arrival time.
- edf: order by `last_compile_start + time_to_burnout`; subject requires a
  deterministic tie-breaker for equal deadlines (e.g. fall back to arrival
  order or coder id).
- This is what a dongle consults when multiple coders are waiting on it.

### 6. Coder thread lifecycle
- One `pthread_create` per coder.
- Loop: acquire both dongles (per your deadlock-avoidance strategy) ->
  log "has taken a dongle" (x2) -> log "is compiling", sleep
  `time_to_compile` -> release both dongles -> log "is debugging", sleep
  `time_to_debug` -> log "is refactoring", sleep `time_to_refactor` -> repeat.
- Update `last_compile_start` the moment compiling actually begins (not when
  requested) since that's what burnout timing is measured from.
- Track `compiles_done`; stop condition #2 is every coder reaching
  `number_of_compiles_required`.

### 7. Monitor thread
- Separate thread (not one of the coder threads).
- Polls/detects when `now - last_compile_start > time_to_burnout` for any
  coder.
- Must log the burnout within 10ms of the real event, then signal all coder
  threads to stop and `pthread_join` them.
- Also responsible for detecting the "everyone hit compiles_required" success
  stop condition.

### 8. Serialized logging
- One mutex guarding all log output so lines from different threads never
  interleave.
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
- Norm: max 25 lines/function, max 4 functions/file, tabs, no `for` loops.
- No crashes/leaks tolerated — any segfault/double-free/etc. is an automatic
  0 during eval.
