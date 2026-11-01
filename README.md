# Unified Engineering Workbench

Public repository: [https://github.com/shivanshOH/RA2311026010197-engineering-workbench](https://github.com/shivanshOH/RA2311026010197-engineering-workbench)

A Windows x64 engineering prototype for the Pass Science Works R&D Internship assignment. The packaged launcher is a native C++17 executable. On double-click it starts a loopback-only C++ service and opens the local workbench in the default browser. The UI needs no account, internet connection, administrator rights, first-run build, or third-party runtime install.

## One-click use
1. Extract the ZIP to any writable folder.
2. Double-click `Launch.exe` at the extracted folder root.
3. Use Geometry, Thermal Analysis, Performance, Hardware, and Backpropagation views. Close the server with **Close workbench** in the page before closing the tab.

The application serves only on `127.0.0.1:8765`. If that port is in use, the launcher shows an actionable message. The UI is local HTML/CSS/JavaScript; all file validation, FEM, benchmark solves, backpropagation and device mock logic runs in C++.

## Architecture
- `src/main.cpp`: C++17 FEM, input parsers, deterministic workload, thread pool, backpropagation and command-line verification tool.
- `src/launcher.cpp`: Windows launcher and local HTTP transport. It serves the bundled UI and calls the C++ core.
- `ui/index.html`: responsive presentation, 3D view controls, charts, status and export. It sends uploaded data to C++ for parsing; rendering only stays in the UI.
- `data/`: self-created ASCII STL and explicitly synthetic CSV plus provenance.
- `results/`: FE output CSV for each requested mesh, backpropagation training CSV/SVG, and reproducible benchmark output CSV after the benchmark is run.
- `tests/`: repeatable Windows input, numerical and parallel checks.
- `docs/`: derivation, assumptions, embedded integration design, AI-use/ownership and run instructions.

## Supported formats and limits
- ASCII STL only. Triangle coordinates are shown as supplied because STL does not reliably specify units. Binary STL and CAD solids are outside scope.
- Nodal CSV header must be exactly `node_id,x_m,y_m,z_m,temperature_C`; all values must be finite. Synthetic sample data is not represented as an ANSYS export.
- Tested maximum input size: 20 MiB per upload/import. Empty, malformed, wrong-schema and non-finite files are rejected with a message.
- No mesh-to-point registration is attempted.

## Thermal model
The solver uses 1 m length, 1e-4 m² area, k=10 W/(m K), q=10,000 W/m³, T(0)=100 °C and T(L)=20 °C. It assembles linear two-node stiffness and uniform generation load terms, applies both prescribed temperatures by elimination, solves the tridiagonal system and checks against `T_exact = 100 - 80x + 500x(1-x)` °C. Midpoint error uses linear FE interpolation. Export uses metre and Celsius headers.

## Performance model
The case generator has fixed seed 20261026, k in [5,25] W/(m K), q in [1000,20000] W/m³ and configurable case/mesh counts. One warm-up is followed by five wall-clock measurements. The report shows median time, T1/Tp, efficiency and max serial/parallel difference. Input creation is outside the timed section; each call creates and joins a bounded worker pool, so measured time includes pool overhead. Results are saved in `results/performance_timings.csv` on the app folder when Run measurements completes.

## Backpropagation
The source implements the four derivatives without automatic differentiation, tests them by central finite differences (epsilon 1e-6, tolerance 1e-5), records 100 simultaneous updates with rate 0.1, and exports the training data and loss plot.

## Embedded design and limits
The documented target set is ST NUCLEO-F401RE, Digilent Arty A7-35T and SEGGER J-Link EDU Mini. All device discovery, connection, telemetry and timeout responses are simulated. The J-Link targets the MCU through the documented SWD path; the Arty is programmed via its own onboard USB-JTAG bridge. No hardware was connected, no wiring performed and no firmware/bitstream deployed. Review `docs/embedded_design.md` before any physical work.

The UI remains responsive because browser requests are asynchronous and server clients run independently. The prototype uses safe completion for running solves; it does not implement cancellation. Benchmark launch-to-ready time is measured through the page's local ready signal. A second concurrent app instance is not supported.

## Build from source (separate from one-click launch)
Prerequisites: Windows 11 x64, Visual Studio 2022 Build Tools with Desktop development with C++, and CMake 3.20+. From the project root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

This builds `build/Release/WorkbenchCLI.exe` and `build/Release/Launch.exe`; CMake copies `ui/` beside `Launch.exe`. The source-build executable may depend on the Visual C++ runtime available in the build environment. The distributed standalone `Launch.exe` is built separately with GCC/libgcc/libstdc++ statically linked and does not need that toolchain at launch. See `dependencies.lock` for versions.

The test runner uses PowerShell:

```powershell
.\tests\run_tests.ps1
```

For direct numerical/parser checks use `WorkbenchCLI.exe --demo`, `--solve N output.csv`, `--import-stl file`, `--import-csv file`, and `--bench cases mesh`.

## Editor run
See `docs/EDITOR_RUN.md`. VS Code requires Microsoft's C/C++ extension only to attach the editor debugger to the same packaged root launcher. The standalone launch has no extension prerequisite.

## Evidence and known limitations
Test output and environment details are in `results/verification_log.txt` and `results/performance_report.md`. The current machine build was exercised, not an independent clean Windows image. No real ANSYS-origin file, physical hardware, handwritten scan or candidate-specific archive name is provided. The handwritten appendix must be copied in the candidate's own handwriting and scanned. The report and package must be reviewed by the candidate before submission.

## Licensing and AI
No third-party implementation library or dataset is bundled. Synthetic examples are original and labeled. GCC and mingw-w64 are build-time tools under their respective licenses; only the statically linked executable is included. See `docs/AI_USE_AND_OWNERSHIP.md` for actual assistance and validation disclosure. Do not claim physical or independent testing that was not performed.
## Evidence and known limitations

Test output and environment details are in `results/verification_log.txt` and `results/performance_report.md`. The development-machine build and app were exercised; no separate clean Windows image was tested. No real ANSYS-origin file, physical hardware, firmware deployment or bitstream is included. Device behavior is simulated. The candidate requested omission of the handwritten appendix; it is not included in this package. The report and package should be reviewed by the candidate before submission.

