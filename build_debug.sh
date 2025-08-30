CPDIRS=(
    "/home/jagger/Projects/Tenkaichi/ModFolder_v2"
    "/mnt/SATAStorage/TenkaichiModding/ModFolder_v2"
)

PROG=$1

x86_64-w64-mingw32-gcc -shared -g -o build/Debug/Afster.dll afs.c afl.c -Wl,--out-implib,build/Debug/libAfster.dll.a -DBUILDING
x86_64-w64-mingw32-gcc "examples/$PROG.c" -o "build/Debug/$PROG.exe" -g -Lbuild/Debug -lAfster

gcc -shared -fPIC -g -o build/Debug/libAfster.so afs.c afl.c
gcc "examples/$PROG.c" -o "build/Debug/$PROG.x86_64" -g -Lbuild/Debug -lAfster -Wl,-rpath,'$ORIGIN'


for DIR in "${CPDIRS[@]}"; do
    cp "$PWD/build/Debug/$PROG.x86_64" "$DIR/tools/AfsCreator/$PROG.x86_64"
	cp "$PWD/build/Debug/$PROG.exe" "$DIR/tools/AfsCreator/$PROG.exe"
done
