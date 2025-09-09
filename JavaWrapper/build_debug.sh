@echo off

GCC=gcc
MINGWGCC=x86_64-w64-mingw32-gcc

if [ ! "libAfster.so" -e ] then
    echo "Afster.dll not found. Compiling it first."
    cd ..
    ./build_debug.sh
    cp ./build/linux/libAfster.so ./JavaWrapper/libAfster.so
    [ $MINGWGCC -n ] && cp ./build/win/Afster.dll ./JavaWrapper/Afster.dll
    cd JavaWrapper
if

echo Compiling Afster Wrapper...

$GCC -shared -fpic -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" jagger_Afster_Afs.c jagger_Afster_Afl.c -L. -lAfster -o libAfsterNative.so
[ $MINGWGCC -n ] && $MINGWGCC -shared -I"$JAVA_HOME\include" -I"$JAVA_HOME\include\win32" jagger_Afster_Afs.c jagger_Afster_Afl.c -L. -lAfster -o AfsterNative.dll

echo "Compiling jagger.Afster package..."
javac -d build src/jagger/Afster/*.java
echo Packaging AfsterJava.jar...
if [ $MINGWGCC -n ]; then
    jar cf AfsterJava.jar -C build jagger libAfster.so libAfsterNative.so Afster.dll AfsterNative.dll
else
    jar cf AfsterJava.jar -C build jagger libAfster.so libAfsterNative.so 
fi

echo Build finished.