#ifndef AFL_H_INCLUDED
#define AFL_H_INCLUDED

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "afs.h"

#define AFL_NAMEBUFFERSIZE AFSMETA_NAMEBUFFERSIZE

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

/** Opens an AFS file and builds the handle for it.
 *
 * @param aflPath path the the AFL file
 * @return Handle to the constructed AFL struct, NULL if it failed.
 */
Afl* afl_open(const char* aflPath);

/** Gets the name of the file with the specified index.
 *  Note that this function does not create a pointer, and thus
 *  does not require the return pointer to be freed.
 *
 * @param afl handle of the AFL
 * @param id index of the file
 * @return char pointer to the name of the file, or NULL if the ID is out of range.
 */
char* afl_getName(Afl* afl, int id);

/** Renames an entry within the AFL file.
 *
 * @param afl handle of the AFL
 * @param id index of the entry
 * @param newName the new name for this entry
 * @return 0 if successful, 1 if AFL is invalid, 2 if given entry ID is invalid.
 */
int afl_rename(Afl* afl, int id, const char* newName);

/** Frees all AFL related memory.
 * @param afl The AFL struct.
 */
void afl_free(Afl* afl);

/** Writes the current state of the AFL to its file.
 *
 * @param afl handle of the AFL
 * @return 0 if successful, 1 if AFL is invalid
 */
int afl_save(Afl* afl);

/** Writes the current state of the AFL to a new file.
 * Note that the filepath MUST be different to the existing file.
 * If not, afs->fstream will be undefined and it will lead to errors.
 *
 * @param afl handle of the AFL
 * @return 0 if successful, 1 if AFL is invalid, 2 if the file can't be written to
 */
int afl_saveNew(Afl* afl, const char* filepath);

/** Imports an AFL Name List into the AFS.
 *
 * @param afs The AFS to be updated
 * @param afl The AFL Name List
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 * @return 0 if successful, 1 if AFS is invalid, 2 if AFL is invalid.
 */
int afl_importAfl(Afl* afl, Afs* afs, bool permament);

#endif // AFL_H_INCLUDED
