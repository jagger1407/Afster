# Afster

A library to edit PS2-Era Dragon Ball games.

AFS Files are archives often used in old games to store models and sounds. This library is meant to help tool creators interface with these files.

Additionally, this library can import AFL (AFS File List) files that house names for each AFS Entry either temporarily in-memory, or permanently within the AFS file itself. For editing of AFL Files, see [this project here](https://github.com/ViveTheModder/afl-editor).

## Usage

A folder with example programs has been packaged with this library to showcase how it can be used. You can compile these using any C compiler. 

An example compilation of [display_afs.c](https://github.com/jagger1407/Afster/blob/main/examples/display_afs.c) would be:
```bash
gcc -o display ../afs.c display_afs.c
```

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
