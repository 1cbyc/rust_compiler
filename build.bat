@echo off
echo Building Rust Compiler in C...

REM Check for different C compilers
where gcc >nul 2>&1
if %errorlevel% == 0 (
    echo Using GCC...
    gcc -Wall -Wextra -std=c99 -g src/*.c -o rust_compiler.exe
    goto :end
)

where cl >nul 2>&1
if %errorlevel% == 0 (
    echo Using Microsoft C/C++ Compiler...
    cl /W3 /std:c11 src/*.c /Fe:rust_compiler.exe
    goto :end
)

where tcc >nul 2>&1
if %errorlevel% == 0 (
    echo Using TinyCC...
    tcc -Wall -g src/*.c -o rust_compiler.exe
    goto :end
)

echo No C compiler found!
echo Please install one of the following:
echo - MinGW-w64 (for gcc)
echo - Visual Studio Build Tools (for cl)
echo - TinyCC (for tcc)
goto :end

:end
if exist rust_compiler.exe (
    echo Build successful! Run with: rust_compiler.exe
) else (
    echo Build failed!
)
pause 