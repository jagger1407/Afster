@echo off

if not exist ".\Afster.dll" (
    echo Afster.dll not found. Compiling it first.
    cd ..
    call .\build_debug.bat
    copy .\build\win\Debug\Afster.dll .\CSharpWrapper
    cd CSharpWrapper
)

echo Compiling Afster Wrapper...

dotnet build -c Release

echo Build finished.
