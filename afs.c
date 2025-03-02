#include "afs.h"

Afs* afs_open(char* filePath) {
    if(filePath == NULL || *filePath == '\0') {
        return NULL;
    }
    FILE* fp = fopen(filePath, "rb+");

    if(fp == NULL) {
        puts("ERROR: afs_open - AFS Filepath invalid.");
        printf("Filepath: %s\n", filePath);
        return NULL;
    }
    Afs* afs = (Afs*)malloc(sizeof(Afs));
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

void afs_free(Afs* afs) {
    if(afs == NULL) {
        puts("WARNING: afs_free - afs pointer already freed. Returning.");
        return;
    }
    free(afs->meta);
    free(afs->header.entryinfo);
    fclose(afs->fstream);
    free(afs);
    afs = NULL;
}

int afs_extractEntryToFile(Afs* afs, int id, const char* output_folderpath) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_extractEntryToFile - Invalid AFS pointer (afs or afs->fstream).");
        return 1;
    }

    if(id < 0 || id > afs->header.entrycount) {
        puts("ERROR: afs_extractEntryToFile - Entry ID out of range.");
        printf("Entry ID: %d\tAFS entry count: %d\n", id, afs->header.entrycount);
        return 2;
    }

    int off = afs->header.entryinfo[id].offset;
    int size = afs->header.entryinfo[id].size;
    u8 buffer[size];

    if(output_folderpath == NULL || *output_folderpath == 0x00 ) {
        puts("ERROR: afs_extractEntryToFile - output_folderpath invalid.");
        return 3;
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
        puts("ERROR: afs_extractEntryToFile - File pointer failed to create.");
        printf("outfile path: %s\n", outpath);
        return 3;
    }

    fseek(afs->fstream, off, SEEK_SET);
    fread(buffer, 1, size, afs->fstream);
    fwrite(buffer, 1, size, outfile);

    fclose(outfile);

    return 0;
}

u8* afs_extractEntryToBuffer(Afs* afs, int id) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_extractEntryToBuffer - Invalid AFS pointer (afs or afs->fstream).");
        return NULL;
    }

    if(id < 0 || id > afs->header.entrycount) {
        puts("ERROR: afs_extractEntryToBuffer - Entry ID out of range.");
        printf("Entry ID: %d\tAFS entry count: %d\n", id, afs->header.entrycount);
        return NULL;
    }

    AfsEntryInfo info = afs->header.entryinfo[id];

    u8* buffer = (u8*)malloc(info.size);
    fseek(afs->fstream, info.offset, SEEK_SET);
    fread(buffer, 1, info.size, afs->fstream);

    return buffer;
}

int afs_extractFull(Afs* afs, const char* output_folderpath) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_extractFull - Invalid AFS File.");
        return 1;
    }
    if(output_folderpath == NULL || *output_folderpath == 0x00 ) {
        puts("ERROR: afs_extractFull - output_folderpath invalid.");
        return 2;
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
            puts("ERROR: afs_extractFull - fp failed to open.");
            printf("filepath: %s\n", filepath);
            continue;
        }
        fseek(afs->fstream, info.offset, SEEK_SET);
        fread(buffer, info.size, 1, afs->fstream);
        fwrite(buffer, info.size, 1, fp);
        free(buffer);

        fclose(fp);
    }
    return 0;
}

int afs_renameEntry(Afs* afs, int id, const char* new_name, bool permanent) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_renameEntry - Invalid AFS File.");
        return 1;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        puts("ERROR: afs_renameEntry - Entry ID out of range.");
        printf("Entry ID: %d, AFS Entry Count: %d\n", id, afs->header.entrycount);
        return 2;
    }
    if(new_name == NULL || *new_name == 0x00) {
        puts("ERROR: afs_renameEntry - new_name is empty/null.");
        return 3;
    }

    AfsEntryInfo metaInf = afs->header.entryinfo[afs->header.entrycount];
    strncpy(afs->meta[id].filename, new_name, AFSMETA_NAMEBUFFERSIZE);

    if(permanent) {
        fseek(afs->fstream, metaInf.offset + sizeof(AfsEntryMetadata) * id, SEEK_SET);
        fwrite(new_name, 1, AFSMETA_NAMEBUFFERSIZE, afs->fstream);
        fseek(afs->fstream, 0, SEEK_SET);
    }

    return 0;
}

Timestamp afs_getLastModifiedDate(Afs* afs, int id) {
    if(id < 0 || id >= afs->header.entrycount) {
        puts("ERROR: afs_getLastModifiedDate - Entry ID out of range.");
        printf("Entry ID: %d\tAFS Entry Count: %d\n", id, afs->header.entrycount);
        Timestamp t = {};
        return t;
    }
    return afs->meta[id].lastModified;
}

char* afs_timestampToString(Timestamp t) {
    char* out = calloc(1, 32);
    sprintf(out, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d", t.day, t.month, t.year, t.hours, t.minutes, t.seconds);
    return out;
}
