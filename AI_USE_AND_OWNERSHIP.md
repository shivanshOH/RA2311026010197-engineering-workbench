# AI-use and ownership disclosure

**Date:** 1 November 2026 (Asia/Kolkata).  
**Assistant:** OpenAI Codex, based on GPT-6. The exact deployed model/version identifier was not exposed in this task interface.  
**External tools:** CMake 4.4.4, GCC 16.2.0/MinGW-w64, PowerShell, Poppler, ReportLab. No paid solver, external AI service call, or third-party source library was used in the packaged application.

## Representative user prompt and work produced
The user asked Codex to review `Assign company.pdf`, complete the assignment and gather what was needed for submission, then explicitly authorized work in the browser and local workspace. Codex extracted the brief and rubric, drafted the C++17 solver, import parsers, benchmark pool and backpropagation, built a local browser UI and Windows launcher, created synthetic sample data, documented the embedded target design, generated report/results, and built a versioned package workspace.

## Critical review and correction
An early AI-generated note reported incorrect initial backpropagation gradients and update values. The C++ central finite-difference checks at epsilon 1e-6 failed to support those values. Recalculation and the passing numerical check established the corrected gradients `(dw,db,dv,dc)=(-1.209464475,-1.209464475,-0.7106820475,-1.537882843)` and corrected simultaneous update. The inaccurate note was corrected in the engineering notes and report. This is an example of an AI result that needed independent checking.

## Validation evidence
CMake configure/build succeeded with GCC 16.2.0. The PowerShell verification script passed: four FEM midpoint errors and refinement ratio; four finite-difference gradient checks; valid STL/CSV imports; rejection of empty, malformed/non-finite STL, wrong CSV schema, non-finite CSV and input over 20 MiB; plus serial/parallel agreement. The local browser UI was launched and inspected; the thermal result, benchmark table and simulated adapter connection/telemetry were exercised. No clean-machine test or physical device test is claimed. Exact outputs are in `results/verification_log.txt` and `results/performance_report.md`.

## Ownership statement
No third-party implementation source or dataset was knowingly reused. The C++ prototype and synthetic STL/CSV examples were drafted for this assignment with AI assistance; the sample CSV is explicitly synthetic. The candidate has not reported personal code edits in this disclosure. Before submitting, the candidate must review and revise the code, report and analysis as needed, understand every claimed result, and add any genuine edits and learning. The handwritten derivation/scan must be produced by the candidate; no handwriting, hardware use or candidate-specific fact is fabricated here.
