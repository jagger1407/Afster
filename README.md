# Afster

A library to edit PS2-Era Dragon Ball games.

AFS Files are archives often used in old games to store models and sounds. This library is meant to help tool creators interface with these files.

Additionally, this library can import AFL (AFS File List) files that house names for each AFS Entry either temporarily in-memory, or permanently within the AFS file itself. For editing of AFL Files, see [this project here](https://github.com/ViveTheModder/afl-editor).

## Usage

An example program + build script has been packaged with this library to showcase how it can be used. It first imports a file list (AFL), then uses it to extract the whole AFS into a folder.
The example program will take 3 arguments:

- arg1 = The AFS file to be extracted.
- arg2 = The AFL file that houses file names or "n" to not import any AFL list.
- arg3 = The path to an output folder, will be created if it doesn't exist yet.

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
- [x] Extract single entries
- [ ] Extract multiple entries
- [x] Extract the whole AFS
- [ ] Change the offset of an entry
- [ ] Change the size of an entry
- [ ] Insert/Append an entry
- [ ] Replace an entry
