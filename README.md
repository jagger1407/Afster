# Afster

A library to edit PS2-Era Dragon Ball games.

AFS Files are archives often used in old games to store models and sounds. This library is meant to help tool creators interface with these files.

Additionally, this library can import AFL (AFS File List) files that house names for each AFS Entry either temporarily in-memory, or permanently within the AFS file itself. For editing of AFL Files, see [this project here](https://github.com/ViveTheModder/afl-editor).

## Status

Currently, the library is able to:

- [x] Parse AFS Files
- [x] Parse AFS Metadata
- [x] Display information about each entry
    - [x] Offset within the AFS file
    - [x] Data size of the Entry
    - [x] Filename (Metadata) - Note: is often empty or incomplete
    - [x] Last Modified date (Metadata)
- [x] Import AFL File
- [x] Rename entry
- [x] Change Metadata of an entry
- [x] Extract single entries
- [ ] Extract multiple entries
- [x] Extract the whole AFS
- [ ] Change the offset of an entry (do we really need this?)
- [ ] Change the size of an entry
- [ ] Insert/Append an entry
- [x] Replace an entry
- [x] Import Folder
- [x] Create an AFS File from a given folder

## Usage
You can find precompiled versions of the example programs in the [releases](https://github.com/jagger1407/Afster/releases/latest) as `examples_win.zip` or `examples_linux.zip`. These are command-line programs to be used inside a console.

#### C/C++
You can download the library and use as-is by including afs.h or afl.h in your code, and compiling it alongside your own code.
Alternatively, a pre-compiled Afster library can be found in the [releases](https://github.com/jagger1407/Afster/releases/latest).

#### Java
An ***experimental*** Afster Wrapper Library for Java can be found in the [releases](https://github.com/jagger1407/Afster/releases/latest). The package name is `jagger.Afster`.

## Compiling

In order to compile this library, I made a build script called `build_debug.bat` (Windows) or `build_debug.sh` (Linux).

#### For Linux:
before you run the build script, open it in a text editor and make sure both `GCC` and `MINGWGCC` are set correctly. 
If you don't wish to compile for Windows at all, just set `MINGWGCC` to nothing or comment it out.

**Syntax:**
`./build_debug.sh <[example program]|all>`
- example programs are found under `./examples/` and must be specified without the file extension.
- if "all" is used, all example programs under `./examples/` will be compiled.
- if nothing is specified, only the Afster library will be compiled.

**Java Wrapper Library:**
The Java wrapper library has its own `build_debug.sh` with the same syntax as the previous one. To use it, simply do
```
cd JavaWrapper
./build_debug.sh
```

#### For Windows:
before you run the build script, open it in a text editor to set the environment correctly.

- COMPILER
    - The compiler you wish to use (only GCC and MSVC are supported currently).
- GCC
    - If you wish to use GCC (e.g. via MSYS2), put the path to gcc.exe in here.
- MSVC
    - If you wish to use MSVC, and you aren't in the VS Developer Console, then this variable must be set to the `vcvarsall.bat` file to get the `cl.exe` location. 

**Syntax:**
`./build_debug.bat <[example program]|all>`
- example programs are found under `./examples/` and must be specified without the file extension.
- if "all" is used, all example programs under `./examples/` will be compiled.
- if nothing is specified, only the Afster library will be compiled.

**Java Wrapper Library:**
The Java wrapper library has its own `build_debug.bat` with the same syntax as the previous one. To use it, simply do
```
cd JavaWrapper
.\build_debug.bat
```