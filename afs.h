#ifndef AFS_H_INCLUDED
#define AFS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

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
/** Size of the buffer from one entry to the next.
 * @note IMPORTANT!!! Must be 16-Byte aligned.
 */
#define AFS_RESERVEDSPACEBUFFER 2048

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
} Afs;

/** opens an AFS file and builds the handle for it.
 *
 * @param filePath path the the AFS file
 * @return Handle to the constructed AFS struct, or NULL if it failed.
 */
Afs* afs_open(char* filePath);

/** With construction comes destruction. This frees all AFS related memory.
 *
 * @param afs The AFS struct to be destroyed.
 */
void afs_free(Afs* afs);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @param output_folderpath The path to the folder where the file should be extracted to.
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range, 3 if output_folderpath is invalid or inaccessible.
 */
int afs_extractEntryToFile(Afs* afs, int id, const char* output_folderpath);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @return A buffer containing the data of the entry, or NULL if there was an error.
 */
u8* afs_extractEntryToBuffer(Afs* afs, int id);

/** Extracts all files within the AFS into a specified folder.
 *
 * @param afs The AFS struct
 * @param output_folderpath The path to the folder where the AFS should be extracted to.
 * @return 0 if successful, 1 if AFS is invalid, 2 if output_folderpath is invalid.
 */
int afs_extractFull(Afs* afs, const char* output_folderpath);

/** Replaces an entry within the AFS without resizing
 * @note DESIGNED FOR INTERNAL USE ONLY
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param data Byte Array containing the new entry data
 * @param data_size Size of the data
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range, 3 if data array is invalid (NULL or zero size).
 */
int _afs_replaceEntry_noResize(Afs* afs, int id, u8* data, int data_size);

/** Clears and resizes the reserved space for the specified entry.
 * This will change each offset for following entries!
 * @note DESIGNED FOR INTERNAL USE ONLY
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param new_size new reserved space for the entry, will be expanded to be 16-Byte aligned.
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range, 3 if AFS_RESERVEDSPACEBUFFER isn't 16-Byte aligned.
 */
int _afs_resizeEntrySpace(Afs* afs, int id, int new_size);

/** Replaces an entry within the AFS.
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param data Byte Array containing the new entry data
 * @param data_size Size of the data
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range, 3 if data array is invalid (NULL or zero size), 4 if resizing was necessary but failed.
 */
int afs_replaceEntry(Afs* afs, int id, u8* data, int data_size);

/** Renames an entry of the AFS.
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param new_name The new name for the entry
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range, 3 if new_name is invalid.
 */
int afs_renameEntry(Afs* afs, int id, const char* new_name, bool permanent);

/** Gets the metadata of a given entry.
 * @param afs The AFS struct
 * @param id The index of the entry
 * @return A copy of the AfsEntryMetaData struct of the entry, or an empty one if the ID is invalid.
 */
AfsEntryMetadata afs_getEntryMetadata(Afs* afs, int id);

/** Sets the metadata of a given entry.
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param new_meta Metadata that will replace the current one
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 * @return 0 if successful, 1 if AFS is invalid, 2 if entry ID is out of range
 */
int afs_setEntryMetadata(Afs* afs, int id, AfsEntryMetadata new_meta, bool permanent);

/** Gets the last modified date of a specific entry in the AFS.
 *
 * @param afs The AFS struct.
 * @param id The index of the entry
 *
 * @return A Timestamp struct that represents the last modified date.
 */
Timestamp afs_getLastModifiedDate(Afs* afs, int id);

/** Converts a Timestamp struct to a string.
 *  Note that a char* is allocated here, and must be manually freed.
 *
 * @param t The Timestamp to be converted.
 * @return C-Style string containing the date.
 */
char* afs_timestampToString(Timestamp t);

#endif // AFS_H_INCLUDED
