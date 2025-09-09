PROG=$1

GCC=gcc
LINUXDIR="./build/linux/Debug"

MINGWGCC=x86_64-w64-mingw32-gcc
WINDIR="./build/win/Debug"

echo "Compiling libAfster.so..."
"$GCC" -shared -fPIC -g -o "$LINUXDIR/libAfster.so" afs.c afl.c
if [ -n "$MINGWGCC" ]; then
    echo "Compiling Afster.dll..."
    "$MINGWGCC" -shared -g -o "$WINDIR/Afster.dll" afs.c afl.c "-Wl,--out-implib,$WINDIR/libAfster.dll.a" -DBUILDING
fi
if [ -n "$PROG" ]; then
    if [ "$PROG" = "all" ]; then
        echo "Compiling all examples..."
        for ex in ./examples/*.c; do
            $GCC "$ex" -o "$LINUXDIR/$(basename "$ex" .c).x86_64" -g "-L$LINUXDIR" -lAfster -Wl,-rpath,'$ORIGIN'
            [ -n "$MINGWGCC" ] && "$MINGWGCC" "$ex" -o "$WINDIR/$(basename "$ex" .c).exe" -g "-L$WINDIR" -lAfster
        done
    else
        echo "Compiling $PROG..."
        $GCC "examples/$PROG.c" -o "$LINUXDIR/$PROG.x86_64" -g "-L$LINUXDIR" -lAfster -Wl,-rpath,'$ORIGIN'
        [ -n "$MINGWGCC" ] && "$MINGWGCC" "examples/$PROG.c" -o "$WINDIR/$PROG.exe" -g "-L$WINDIR" -lAfster
    fi
fi
