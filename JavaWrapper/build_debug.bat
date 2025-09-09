@echo off

rem Possible Values: "GCC", "MSVC"
set "COMPILER=GCC"

rem This is MY setup. Please change this to fit your personal setups.
set "GCC=C:\msys64\clang64\bin\gcc.exe"
set MSVC="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

if not exist ".\Afster.dll" (
    echo Afster.dll not found. Compiling it first.
    cd ..
    call .\build_debug.bat
    copy .\build\win\Debug\Afster.dll .\JavaWrapper
    cd JavaWrapper
)

echo Compiling Afster Wrapper...

if "%COMPILER%"=="GCC" goto :compilingGCC
if "%COMPILER%"=="MSVC" goto :compilingMSVC 

:compilingGCC
%GCC% -shared -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" jagger_Afster_Afs.c jagger_Afster_Afl.c -L. -lAfster -o AfsterNative.dll
goto :endif_23

:compilingMSVC
for /f "delims=" %%i in ('cl') do set CL=%%i
if "%CL%"=="" (
    call %MSVC% x64
)

cl /I"%JAVA_HOME%\include" /I"%JAVA_HOME%\include\win32" /LD jagger_Afster_Afs.c jagger_Afster_Afl.c Afster.dll /FeAfsterNative.dll
goto :endif_23

:endif_23
echo Compiling jagger.Afster package...
javac -d build src/jagger/Afster/*.java
echo Packaging AfsterJava.jar...
jar cf AfsterJava.jar -C build jagger Afster.dll AfsterNative.dll

echo Build finished.