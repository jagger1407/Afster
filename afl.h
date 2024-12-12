#ifndef AFL_H_INCLUDED
#define AFL_H_INCLUDED

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define AFL_NAMEBUFFERSIZE 0x20

#define AFL_NAME(afl, x) (afl->filenames + (x * AFL_NAMEBUFFERSIZE))

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
} Afl_t;

/** opens an AFS file and builds the handle for it.
 *
 * @param aflPath path the the AFL file
 * @return Handle to the constructed AFL struct, NULL if it failed.
 */
Afl_t* openAfl(const char* aflPath);

/** Gets the name of the file with the specified index.
 *  Note that this function does not create a pointer, and thus
 *  does not require the return pointer to be freed.
 *
 * @param afl path the the AFL file
 * @param id index of the file
 * @return char pointer to the name of the file.
 */
char* getAflName(Afl_t* afl, int id);

/** Frees all AFL related memory.
 * @param afl The AFL struct.
 */
void freeAfl(Afl_t* afl);

#endif // AFL_H_INCLUDED
