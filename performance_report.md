# Performance run record

Observed 2026-11-01 on the development laptop. The app was launched and benchmarked through its local browser interface. Workload: 50,000 fixed-seed cases, 40 elements/case, one warm-up and five measured runs per worker count. The timing includes worker-pool creation/join and excludes input generation and file I/O.

| Workers | Median (ms) | Speedup | Efficiency | Raw measured values (ms) |
|---:|---:|---:|---:|---|
| 1 | 184.1098 | 1.000x | 100.0% | 181.1987, 189.5121, 184.1098, 179.4551, 193.2613 |
| 2 | 136.8751 | 1.345x | 67.3% | 136.8751, 130.8149, 138.7848, 136.3219, 137.2829 |
| 3 | 112.5725 | 1.635x | 54.5% | 99.9489, 112.3555, 120.0406, 120.3379, 112.5725 |
| 4 | 138.5590 | 1.329x | 33.2% | 131.3783, 140.6810, 138.5590, 134.1457, 138.6416 |

Maximum serial/parallel temperature difference: 0 C for every worker count. Host: Intel Core i7-12700H, 14 physical cores / 20 logical CPUs, 15.64 GiB installed RAM. Compiler: GCC 16.2.0 C++17, `-O2 -static -static-libgcc -static-libstdc++`. Local ready signal was reached 796.7608 ms after process start on this observation. This value is a one-off launcher-start observation, not a statistical measure. The raw CSV with the full metadata and repetitions is `performance_timings.csv`.

Three workers were fastest in this single run at 1.635x serial. Four workers regressed relative to three, showing that this workload does not scale monotonically. Results depend on operating-system scheduling, temperature, and concurrent load.
