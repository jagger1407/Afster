@echo off

rem Possible Values: "GCC", "MSVC"
set "COMPILER=GCC"

rem This is MY setup. Please change this to fit your personal setups.
set "GCC=C:\msys64\clang64\bin\gcc.exe"
set MSVC="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

rem This is the directory where the compiled files will be stored.
set "DIR=.\build\win\Debug"

echo Compiling Afster Library...

if "%COMPILER%"=="GCC" goto :compilingGCC
if "%COMPILER%"=="MSVC" goto :compilingMSVC 

:compilingGCC
"%GCC%" -shared -g -o "%DIR%\Afster.dll" afs.c afl.c "-Wl,--out-implib,%DIR%\libAfster.dll.a" -DBUILDING

if NOT "%1"=="" (
    if "%1"=="all" (
        echo Compiling all examples...
        for %%f in (".\examples\*.c") do (
            "%GCC%" ".\examples\%%~nxf" -o "%DIR%\%%~nf.exe" -g "-L%DIR%" -lAfster
        )
    ) else (
        echo Compiling %1...
        "%GCC%" ".\examples\%1.c" -o "%DIR%\%1.exe" -g "-L%DIR%" -lAfster
    )
    
)
goto :end

:compilingMSVC

for /f "delims=" %%i in ('cl') do set CL=%%i
if "%CL%"=="" (
    call %MSVC% x64
)

cl /LD /Od /Zi /D BUILDING afs.c afl.c /Fe"%DIR%\Afster.dll" /IMPLIB:"%DIR%\Afster.lib"

if NOT "%1"=="" (
    echo Compiling %1...
    cl ".\examples\%1.c" /Fe"%DIR%\%1.exe" /Zi /Od /link /LIBPATH:"%DIR%" Afster.lib
    del %1%.obj
)
del afs.obj
del afl.obj
del .\*.pdb
goto :end

:end
echo Build finished.