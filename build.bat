@echo off
echo ================================================
echo   Building Lab1_RK4.exe  (MSVC)
echo ================================================
echo.

set PYTHON_EXE=D:\nm26\cpp\.venv\Scripts\python.exe

if not exist "%PYTHON_EXE%" (
    echo ERROR: Python not found at %PYTHON_EXE%
    echo Edit PYTHON_EXE in this bat file to point to your python.exe
    pause & exit /b 1
)
echo Python: %PYTHON_EXE%

call "D:\vsss22\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment.
    pause & exit /b 1
)

echo.
echo [1/3] Compiling C++ with MSVC...
cl /O2 /std:c++17 /EHsc /Fe:lab1.exe main.cpp integrator.cpp models.cpp results.cpp /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo ERROR: C++ compilation failed.
    pause & exit /b 1
)
echo       lab1.exe - OK

echo.
echo [2/3] Checking PyInstaller...
"%PYTHON_EXE%" -m pip show pyinstaller >nul 2>&1
if errorlevel 1 (
    echo       Installing PyInstaller...
    "%PYTHON_EXE%" -m pip install pyinstaller
    if errorlevel 1 (
        echo ERROR: PyInstaller install failed.
        pause & exit /b 1
    )
)
echo       PyInstaller - OK

echo.
echo [3/3] Building GUI exe...
"%PYTHON_EXE%" -m PyInstaller --onefile --noconsole --name "Lab1_RK4" --add-binary "lab1.exe;." --hidden-import matplotlib.backends.backend_tkagg gui.py
if errorlevel 1 (
    echo ERROR: PyInstaller failed.
    pause & exit /b 1
)

echo.
echo ================================================
echo   Done!  dist\Lab1_RK4.exe
echo ================================================
echo.
pause
