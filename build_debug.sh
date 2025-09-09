PROG=$1

GCC=gcc
LINUXDIR="./build/linux/Debug"

MINGWGCC=x86_64-w64-mingw32-gcc
WINDIR="./build/win/Debug"

echo "Compiling libAfster.so..."
$GCC -shared -fPIC -g -o "$LINUXDIR/libAfster.so" afs.c afl.c
if [ $MINGWGCC -n ]; then
    echo "Compiling Afster.dll..."
    $MINGWGCC -shared -g -o "$WINDIR/Afster.dll" afs.c afl.c "-Wl,--out-implib,$WINDIR/libAfster.dll.a" -DBUILDING
fi
if [ $PROG -n ]; then
    if [ $PROG == "all" ]; then
        echo "Compiling all examples..."
        for ex in ./examples/*.c; do
            $GCC "examples/$PROG.c" -o "$LINUXDIR/$PROG.x86_64" -g "-L$LINUXDIR" -lAfster -Wl,-rpath,'$ORIGIN'
            [ $MINGWGCC -n ] && $MINGWGCC "examples/$PROG.c" -o "$WINDIR/$PROG.exe" -g "-L$WINDIR" -lAfster
        done
    else
        echo "Compiling $PROG..."
        $GCC "examples/$PROG.c" -o "$LINUXDIR/$PROG.x86_64" -g "-L$LINUXDIR" -lAfster -Wl,-rpath,'$ORIGIN'
        [ $MINGWGCC -n ] && $MINGWGCC "examples/$PROG.c" -o "$WINDIR/$PROG.exe" -g "-L$WINDIR" -lAfster
    fi
fi
