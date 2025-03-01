#ifndef AFL_H_INCLUDED
#define AFL_H_INCLUDED

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define AFL_NAMEBUFFERSIZE 0x20

#define _AFL_NAME(afl, x) (afl->filenames + (x * AFL_NAMEBUFFERSIZE))

typedef struct {
    char identifier[4];
    u32 unknown1;
    s32 unknown2;
    u32 filecount;
} AflHeader;

typedef struct {
    AflHeader head;
    char* filenames;
    FILE* fstream;
} Afl;

/** opens an AFS file and builds the handle for it.
 *
 * @param aflPath path the the AFL file
 * @return Handle to the constructed AFL struct, NULL if it failed.
 */
Afl* afl_open(const char* aflPath);

/** Gets the name of the file with the specified index.
 *  Note that this function does not create a pointer, and thus
 *  does not require the return pointer to be freed.
 *
 * @param afl path the the AFL file
 * @param id index of the file
 * @return char pointer to the name of the file, or NULL if the ID is out of range.
 */
char* afl_getName(Afl* afl, int id);

/** Frees all AFL related memory.
 * @param afl The AFL struct.
 */
void afl_free(Afl* afl);

#endif // AFL_H_INCLUDED
