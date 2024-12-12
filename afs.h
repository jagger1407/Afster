#ifndef AFS_H_INCLUDED
#define AFS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "afl.h"

#ifdef _WIN32

#include <io.h>
#define access(x, y) _access(x, y)
#define F_OK 0

#include <direct.h>
#define mkdir(path) _mkdir(path)

#elif defined(__unix__)

#include <unistd.h>

#include <sys/stat.h>
#define mkdir(x) mkdir(x, 0777)

#endif

typedef struct {
    u16 year;
    u16 month;
    u16 day;
    u16 hours;
    u16 minutes;
    u16 seconds;
} Timestamp;

typedef struct {
    u32 offset;
    u32 size;
} AfsEntryInfo;

typedef struct {
    char identifier[4];
    u32 entrycount;
    AfsEntryInfo* entryinfo;
} AfsHeader;

#define AFSMETA_NAMEBUFFERSIZE 0x20

typedef struct {
    char filename[AFSMETA_NAMEBUFFERSIZE];
    Timestamp lastModified;
    u32 unknown;    // I genuinely have no idea, maybe unused
}AfsEntryMetadata;

typedef struct {
    AfsHeader header;
    AfsEntryMetadata* meta;
    //AfsEntryInfo* sortedEntries;
    FILE* fstream;
} Afs_t;

/** opens an AFS file and builds the handle for it.
 *
 * @param filePath path the the AFS file
 * @return Handle to the constructed AFS struct, NULL if it failed.
 */
Afs_t* openAfs(char* filePath);

/** Imports an AFL Name List into the AFS.
 *
 * @param afs The AFS to be updated
 * @param afl The AFL Name List
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 */
void importAfl(Afs_t* afs, Afl_t* afl, bool permament);

/** With construction comes destruction. This frees all AFS related memory.
 *
 * @param afs The AFS struct to be destroyed.
 */
void freeAfs(Afs_t* afs);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @param output_folderpath The path to the folder where the file should be extracted to.
 */
void extractEntryToFile(Afs_t* afs, int id, const char* output_folderpath);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @return A buffer containing the data of the entry.
 */
u8* extractEntryToBuffer(Afs_t* afs, int id);

/** Extracts all files within the AFS into a specified folder.
 *
 * @param afs The AFS struct
 * @param output_folderpath The path to the folder where the AFS should be extracted to.
 */
void extractWholeAfs(Afs_t* afs, const char* output_folderpath);

/** Gets the last modified date of a specific entry in the AFS.
 *
 * @param afs The AFS struct.
 * @param id The index of the entry
 *
 * @return A Timestamp struct that represents the last modified date.
 */
Timestamp getLastModifiedDate(Afs_t* afs, int id);

/** Converts a Timestamp struct to a string.
 *  Note that a char* is allocated here, and must be manually freed.
 *
 * @param t The Timestamp to be converted.
 * @return C-Style string containing the date.
 */
char* timestampToString(Timestamp t);

#endif // AFS_H_INCLUDED
