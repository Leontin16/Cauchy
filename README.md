# Lab 1 — Numerical Solution of the Cauchy Problem

A GUI application for solving ODEs using the **4th-order Runge-Kutta method**, built for a numerical methods course. The computational core is written in C++; the interface and plots are handled by Python.

---

## Problems Solved

### Test Problem
$$\frac{du}{dx} = -1.5\,u, \quad u(0) = u_0$$

Exact solution: $u(x) = u_0 \cdot e^{-1.5x}$. Used to validate the method and compare numerical vs. exact results.

### Main Problem — Nonlinear Oscillator
$$m\,u'' + c\,u' + k\,u + k^*u^3 = 0$$

| Parameter | Value |
|-----------|-------|
| $m$ | 0.01 N·s²/cm |
| $c$ | 0.15 N·s/cm² |
| $k$ | 2.0 N/cm |
| $k^*$ | 2.0 N/cm³ |
| $u(0)$ | 10 cm |
| $u'(0)$ | 0 |

---

## Features

- **Constant step** RK4 with local error estimation (double step comparison)
- **Adaptive step** RK4 with automatic step halving / doubling based on tolerance $\varepsilon$
- Output table: $x_i$, $v_i$, $v2_i$, $v_i - v2_i$, LTE, $h_i$, step-halving counter C1, step-doubling counter C2
- For the test problem: exact solution $u_i$ and global error $|u_i - v_i|$
- Statistics: $n$, $\max|\text{LTE}|$, $\max h_i$, $\min h_i$, total halvings/doublings
- **Three plot tabs**: solution graph, phase portrait, error plot

---

## Architecture

```
Lab1_RK4.exe  (final build)
├── gui.py        — Python/Tkinter UI + Matplotlib plots
└── lab1.exe      — C++ solver (RK4 engine)
```

Python communicates with the C++ binary via **subprocess**: parameters are passed through stdin, the table output is captured from stdout, and plot data is read from a `.txt` file written by the C++ side.

```
GUI  ──stdin──▶  lab1.exe  ──stdout──▶  table display
                    │
                    └──── *.txt ──▶  matplotlib plots
```

---

## Project Structure

```
.
├── main.cpp          # Entry point, user input, file output
├── integrator.cpp/hpp  # RK4 step, fixed and adaptive integration
├── models.cpp/hpp    # RHS functions for both problems
├── results.cpp/hpp   # Table and statistics output
├── utils.hpp         # Common includes
├── gui.py            # Python GUI
├── build.bat         # One-click build script (MSVC + PyInstaller)
└── README.md
```

---

## Requirements

**To build:**
- Visual Studio 2022 with the **"Desktop development with C++"** workload
- Python 3.10+ with a virtual environment containing:
  - `matplotlib`
  - `numpy`
  - `pyinstaller` (installed automatically by `build.bat`)

**To run the pre-built exe:**
- Windows 10/11 x64
- No additional dependencies required

---

## Build

1. Clone the repository and open a terminal in the project folder
2. Activate your Python virtual environment:
   ```powershell
   .\.venv\Scripts\Activate.ps1
   ```
3. Open `build.bat` and set the two paths at the top to match your system:
   ```bat
   set PYTHON_EXE=D:\nm26\cpp\.venv\Scripts\python.exe
   call "D:\vsss22\VC\Auxiliary\Build\vcvarsall.bat" x64 # adjust to your system
   ```
4. Run the build script:
   ```powershell
   .\build.bat
   ```
5. The finished executable will be at `dist\Lab1_RK4.exe`

---

## Usage

Launch `Lab1_RK4.exe` (or `python gui.py` during development).

1. Select the **problem** (Test / Main)
2. Select the **method** (Constant step / Adaptive step)
3. Fill in the parameters: $a$, $b$, $h_0$, $N_{\max}$, and $\varepsilon$ or $u_0$ as needed
4. Click **▶ Run** — the table appears in the first tab, plots in the remaining three

---

## Output Files

After each run, the C++ solver writes a data file to the working directory:

| File | Contents |
|------|----------|
| `test_const.txt` | Test problem, constant step |
| `test_adapt.txt` | Test problem, adaptive step |
| `main_const.txt` | Main problem, constant step |
| `main_adapt.txt` | Main problem, adaptive step |
