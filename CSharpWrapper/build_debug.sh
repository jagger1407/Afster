GCC=gcc
MINGWGCC=x86_64-w64-mingw32-gcc

if [ ! -e "libAfster.so" ]; then
    echo "Afster.dll not found. Compiling it first."
    cd ..
    ./build_debug.sh
    cp ./build/linux/Debug/libAfster.so ./CSharpWrapper/libAfster.so
    [ -n "$MINGWGCC" ] && cp ./build/win/Debug/Afster.dll ./CSharpWrapper/Afster.dll
    cd CSharpWrapper
fi

echo "Compiling Afster Wrapper..."

dotnet build -c Debug

echo "Build finished."
