#ifndef AFS_H_INCLUDED
#define AFS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>

#include "types.h"

#ifdef _WIN32

#include <windows.h>

#include <io.h>
#define access(x, y) _access(x, y)
#define F_OK 0

#define PATH_SEP '\\'

#include <direct.h>
#define mkdir(path) _mkdir(path)

#endif
#ifdef __unix__

#include <unistd.h>

#define PATH_SEP '/'

#include <sys/stat.h>
#include <fcntl.h>
#define mkdir(x) mkdir(x, 0777)

#include <time.h>

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
    u32 filesize;    // Seems to verify the file size? Not sure about this one
}AfsEntryMetadata;

typedef struct {
    AfsHeader header;
    AfsEntryMetadata* meta;
    FILE* fstream;
} Afs;

/** opens an AFS file and builds the handle for it.
 *
 * @param filePath path the the AFS file
 *
 * @retval Handle to the constructed AFS struct.
 * @retval NULL if it failed.
 */
EXPORT Afs* afs_open(char* filePath);

/** With construction comes destruction. This frees all AFS related memory.
 *
 * @param afs The AFS struct to be destroyed.
 */
EXPORT void afs_free(Afs* afs);

/** Gets the total amount of entries within this AFS. 
 * 
 * @param afs The AFS struct
 * @return The total entry count (EXCLUDING Metadata section).
 */
EXPORT int afs_getEntrycount(Afs* afs);

/** Gets the entry info for a specified entry within the AFS. 
 * 
 * @param afs The AFS struct
 * @param id The index of the entry
 * @return A copy of the AfsEntryInfo struct for the specified entry.
 * Zeroed out if an error occurs.
 */
EXPORT AfsEntryInfo afs_getEntryinfo(Afs* afs, int id);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @param folderpath The path to the folder where the file should be extracted to.
 * @param filepath (Optional) if a pointer is given, the filepath of the output file will be copied.
 *
 * @retval 0 on successful extraction.
 * @retval 1 if the AFS Handle or file stream is NULL.
 * @retval 2 if the Entry ID is out of range.
 * @retval 3 if the folderpath is invalid.
 * @retval 4 if the file couldn't be created.
 * @note The filepath buffer must be large enough to store the filepath
 */
EXPORT int afs_extractEntryToFile(Afs* afs, int id, const char* folderpath, char* filepath);

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 *
 * @retval A buffer containing the data of the entry.
 * @retval NULL if there was an error.
 */
EXPORT u8* afs_extractEntryToBuffer(Afs* afs, int id);

/** Frees a buffer allocated by one of the functions within this library.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @retval A buffer containing the data of the entry.
 * @retval NULL if there was an error.
 * @note Only added for usage with the C# Wrapper Library.
 */
EXPORT void afs_freeBuffer(void* buffer);

/** Extracts all files within the AFS into a specified folder.
 *
 * @param afs The AFS struct
 * @param folderpath The path to the folder where the AFS should be extracted to.
 * @retval 0 on successful extraction
 * @retval 1 if AFS is invalid
 * @retval 2 if folderpath is invalid.
 */
EXPORT int afs_extractFull(Afs* afs, const char* folderpath);

/** Replaces an entry within the AFS.
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param data Byte Array containing the new entry data
 * @param data_size Size of the data
 *
 * @retval 0 if the operation was successful.
 * @retval 1 if the AFS is invalid.
 * @retval 2 if the entry ID is out of range.
 * @retval 3 if the data array is invalid (NULL or zero size).
 * @retval 4 if resizing was necessary but failed.
 */
EXPORT int afs_replaceEntry(Afs* afs, int id, u8* data, int data_size);

/** Replaces multiple entries in the AFS with given files.
 *
 * @param afs The AFS struct
 * @param entries An array containing all entry IDs that should be replaced (Entries marked -1 will be skipped)
 * @param filepaths An array containing all file paths for those entries
 * @param amount_entries The total amount of entries that should be replaced.
 * @note entries and filepaths should have the same size.
 *
 * @retval 0 if the operation was successful.
 * @retval 1 if the AFS is invalid.
 * @retval 2 if there was an issue with the passed arrays.
 * @retval 3 if amount_entries is invalid.
 */
EXPORT int afs_replaceEntriesFromFiles(Afs* afs, int* entries, char** filepaths, int amount_entries);

/** Renames an entry of the AFS.
 *
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param new_name The new name for the entry
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 *
 * @retval 0 if the operation was successful.
 * @retval 1 if the AFS is invalid.
 * @retval 2 if entry ID is out of range.
 * @retval 3 if new_name is invalid.
 */
EXPORT int afs_renameEntry(Afs* afs, int id, const char* new_name, bool permanent);

/** Gets the metadata of a given entry.
 * @param afs The AFS struct
 * @param id The index of the entry
 * @return A copy of the AfsEntryMetaData struct of the entry, or an empty struct if the ID is invalid.
 */
EXPORT AfsEntryMetadata afs_getEntryMetadata(Afs* afs, int id);

/** Sets the metadata of a given entry.
 * @param afs The AFS struct
 * @param id The index of the entry
 * @param new_meta Metadata that will replace the current one
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 *
 * @retval 0 if successful.
 * @retval 1 if the AFS is invalid.
 * @retval 2 if the entry ID is out of range.
 */
EXPORT int afs_setEntryMetadata(Afs* afs, int id, AfsEntryMetadata new_meta, bool permanent);

/** Gets the last modified date of a specific entry in the AFS.
 *
 * @param afs The AFS struct.
 * @param id The index of the entry
 *
 * @return A Timestamp struct that represents the last modified date.
 */
EXPORT Timestamp afs_getLastModifiedDate(Afs* afs, int id);

/** Converts a Timestamp struct to a string.
 *
 * @param t The Timestamp to be converted.
 * @return C-Style string containing the date.
 *
 * @note The output string must be manually freed!
 */
EXPORT char* afs_timestampToString(Timestamp t);

#endif // AFS_H_INCLUDED
