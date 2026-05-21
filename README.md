# Flush+Reload PoC

**Purpose**: Demonstration of a simple cache-based side-channel (flush+reload) proof-of-concept. The repository contains a `victim` that repeatedly accesses a random cache line in a shared memory file and an `attacker` that detects which line was accessed by flushing cache lines and timing reloads.

**Files**
- **`victim.c`**: victim program that mmaps `shared_file`, selects a random slot, and continuously reads from that slot.
- **`attacker.c`**: attacker program that flushes candidate cache lines with `clflush`, waits briefly, then measures access times with `rdtsc` to find the fastest (most likely accessed) slot.
- **`shared_file`**: a zero-filled shared file used for memory mapping. If missing, create it as shown below.

**Build**
Run these commands in the repository root:
```bash
gcc -std=c11 -O2 -Wall -Wextra victim.c -o victim
gcc -std=c11 -O2 -Wall -Wextra attacker.c -o attacker
```

**Create shared file (if missing)**
```bash
dd if=/dev/zero of=shared_file bs=1 count=131072
chmod 666 shared_file
```

**Run**
1. Start the victim (background recommended):
```bash
nohup ./victim > victim.log 2>&1 & echo $! > victim.pid
```
2. Run the attacker in another terminal:
```bash
./attacker
```
3. Stop the victim when done:
```bash
kill $(cat victim.pid) || pkill victim
rm -f victim.pid
```

**How it works (high level)**
- Victim: mmaps `shared_file` and chooses a random slot index in `[0, 1024)`. It repeatedly reads the byte at `slot * 128` to create a cache hit on that line.
- Attacker: for each epoch it:
  - Flushes all candidate cache lines with `_mm_clflush`.
  - Sleeps briefly to let the victim run.
  - Measures the time to read each candidate line using `__rdtsc()`; a significantly smaller read time indicates the line was loaded into cache (victim likely accessed it).

**Notes & limitations**
- Timing and isolation matter: results can be noisy on virtualized or heavily loaded systems.
- The attacker sample code is basic — it finds the single fastest slot per epoch but does not apply statistical filtering or thresholds.
- Uses x86-specific intrinsics (`__rdtsc`, `_mm_clflush`) so it is x86-only.

**Next steps (optional)**
- Add aggregation/statistics to reduce noise and false positives.
- Improve synchronization between victim and attacker (e.g., shared signaling) for deterministic tests.
- Add a README section describing expected timing ranges on your hardware.

If you want, I can add an expanded README section with example output and interpretation, or implement better detection heuristics.
# flush-reload