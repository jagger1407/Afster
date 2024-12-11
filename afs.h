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
Afs_t* openAfs(char* filePath) {
    if(filePath == nullptr || *filePath == '\0') {
        return NULL;
    }
    FILE* fp = fopen(filePath, "rb+");

    if(fp == NULL) {
        puts("ERROR Afs::openAfs - AFS Filepath invalid.");
        printf("Filepath: %s\n", filePath);
        return NULL;
    }
    Afs_t* afs = (Afs_t*)malloc(sizeof(Afs_t));
    afs->fstream = fp;

    // Read AFS Header
    fread(&afs->header, 8, 1, afs->fstream);
    AfsHeader* head = &afs->header;

    // Read Info for all files in the AFS
    head->entryinfo = (AfsEntryInfo*)malloc((head->entrycount + 1) * sizeof(AfsEntryInfo));
    fread(head->entryinfo, sizeof(AfsEntryInfo), head->entrycount + 1, afs->fstream);

    // Read Metadata for all files in the AFS
    int metaSize = head->entryinfo[head->entrycount].size;
    afs->meta = (AfsEntryMetadata*)malloc(metaSize);
    fseek(afs->fstream, head->entryinfo[head->entrycount].offset, SEEK_SET);
    fread(afs->meta, metaSize, 1, afs->fstream);

    return afs;
}

/** Imports an AFL Name List into the AFS.
 *
 * @param afs The AFS to be updated
 * @param afl The AFL Name List
 * @param permanent If true, the function will overwrite the metadata in the AFS File itself as well.
 */
void importAfl(Afs_t* afs, Afl_t* afl, bool permament) {
    if(afs == NULL) {
        puts("ERROR: Afs::importAfl - afs null.");
        return;
    }
    if(afs->fstream == NULL) {
        puts("ERROR: Afs::importAfl - afs exists, but afs->fstream doesn't.");
        return;
    }
    if(afl == NULL) {
        puts("ERROR: Afs::importAfl - afl null.");
        return;
    }

    for(int i=0;i<afl->head.filecount;i++) {
        memcpy(afs->meta[i].filename, AFL_NAME(i), AFSMETA_NAMEBUFFERSIZE);
    }
    if(permament) {
        AfsEntryInfo metaInfo = afs->header.entryinfo[afs->header.entrycount];
        fseek(afs->fstream, metaInfo.offset, SEEK_SET);
        fwrite(afs->meta, metaInfo.size, 1, afs->fstream);
    }
}

/** With construction comes destruction. This frees all AFS related memory.
 *
 * @param afs The AFS struct to be destroyed.
 */
void freeAfs(Afs_t* afs) {
    free(afs->meta);
    free(afs->header.entryinfo);
    fclose(afs->fstream);
    free(afs);
}

/** Extracts a singular file from the AFS to the specified folder.
 *
 * @param afs The AFS struct
 * @param id The index of the extracted file
 * @param output_folderpath The path to the folder where the file should be extracted to.
 */
void extractFile(Afs_t* afs, int id, const char* output_folderpath) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR Afs::extractFile - Invalid AFS pointer (afs or afs->fstream).");
        return;
    }

    int off = afs->header.entryinfo[id].offset;
    int size = afs->header.entryinfo[id].size;
    u8 buffer[size];

    if(output_folderpath == NULL || *output_folderpath == 0x00 ) {
        puts("ERROR Afs::extractFile - output_folderpath invalid.");
        return;
    }

    char outpath[strlen(output_folderpath)+AFSMETA_NAMEBUFFERSIZE];
    strncpy(outpath, output_folderpath, 100);

    if(*afs->meta[id].filename != 0x00) {
        strncat(outpath, afs->meta[id].filename, AFSMETA_NAMEBUFFERSIZE);
    }
    else {
        char temp[AFSMETA_NAMEBUFFERSIZE];
        snprintf(temp, AFSMETA_NAMEBUFFERSIZE, "blank_%d", id);
        strncat(outpath, temp, AFSMETA_NAMEBUFFERSIZE);
    }

    FILE* outfile = fopen(outpath, "wb");

    if(outfile == NULL) {
        puts("ERROR Afs::extractFile - File pointer failed to create.");
        printf("outfile path: %s\n", outpath);
        return;
    }

    fseek(afs->fstream, off, SEEK_SET);
    fread(buffer, 1, size, afs->fstream);
    fwrite(buffer, 1, size, outfile);

    fclose(outfile);
}

/** Extracts all files within the AFS into a specified folder.
 *
 * @param afs The AFS struct
 * @param output_folderpath The path to the folder where the AFS should be extracted to.
 */
void extractWholeAfs(Afs_t* afs, const char* output_folderpath) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR Afs::extractWholeAfs - Invalid AFS File.");
        return;
    }
    if(output_folderpath == NULL || *output_folderpath == 0x00 ) {
        puts("ERROR Afs::extractWholeAfs - output_folderpath invalid.");
        return;
    }

    // In order to ensure each file path will be concatenated correctly,
    // we must ensure the folder path ends with a '/' character.
    int pathlen = strlen(output_folderpath);
    char folderpath[pathlen+2];
    memset(folderpath, 0x00, pathlen+2);
    strcpy(folderpath, output_folderpath);
    if(output_folderpath[pathlen-1] != '/') {
        folderpath[pathlen++] = '/';
    }

    if(access(folderpath, F_OK) != 0) {
        mkdir(folderpath);
    }

    for(int i=0;i<afs->header.entrycount;i++) {
        AfsEntryInfo info = afs->header.entryinfo[i];
        AfsEntryMetadata meta = afs->meta[i];
        char filepath[sizeof(folderpath)+AFSMETA_NAMEBUFFERSIZE];
        strcpy(filepath, folderpath);

        char name[AFSMETA_NAMEBUFFERSIZE];
        if(*meta.filename == 0x00) {
            snprintf(name, AFSMETA_NAMEBUFFERSIZE, "blank_%d", i);
        }
        else {
            memcpy(name, meta.filename, AFSMETA_NAMEBUFFERSIZE);
        }
        strncat(filepath, name, AFSMETA_NAMEBUFFERSIZE);

        int fcnt = 1;
        // file already exists
        while(access(filepath, F_OK) == 0) {
            // we need to check whether this file has an extension
            char* ext = strrchr(name, '.');
            if(ext != NULL) {
                char tmp[ext-name+1];
                strncpy(tmp, name, ext-name);
                tmp[ext-name] = 0x00;
                snprintf(filepath + pathlen, AFSMETA_NAMEBUFFERSIZE, "%s(%d)%s", tmp, fcnt++, ext);
            }
            else {
                char tmp[0x30];
                snprintf(tmp, 0x30, "%s(%d)", name, fcnt++);
                strcpy(filepath+pathlen, tmp);
            }
        }

        u8* buffer = (u8*)malloc(info.size);
        FILE* fp = fopen(filepath, "wb");
        if(fp == NULL) {
            puts("ERROR: Afs::extractWholeAfs - fp failed to open.");
            printf("filepath: %s\n", filepath);
            continue;
        }
        fseek(afs->fstream, info.offset, SEEK_SET);
        fread(buffer, info.size, 1, afs->fstream);
        fwrite(buffer, info.size, 1, fp);
        free(buffer);

        fclose(fp);
    }
}

/** Gets the last modified metadata as a string.
 *
 * @param meta Metadata of the AFS Entry.
 * @return C-Style string containing the date.
 */
char* getLastModifiedDate(AfsEntryMetadata* meta) {
    char* out = calloc(1, 32);
    sprintf(out, "%02d.%02d.%4d %02d:%02d:%02d", meta->lastModified.day, meta->lastModified.month, meta->lastModified.year, meta->lastModified.hours, meta->lastModified.minutes, meta->lastModified.seconds);
    return out;
}

#endif // AFS_H_INCLUDED
