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

int afs_getEntrycount(Afs* afs) {
    return afs->header.entrycount;
}

char* afs_extractEntryToFile(Afs* afs, int id, const char* output_folderpath) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_extractEntryToFile - Invalid AFS pointer (afs or afs->fstream).");
        return NULL;
    }

    if(id < 0 || id > afs->header.entrycount) {
        puts("ERROR: afs_extractEntryToFile - Entry ID out of range.");
        printf("Entry ID: %d\tAFS entry count: %d\n", id, afs->header.entrycount);
        return NULL;
    }

    if(output_folderpath == NULL || *output_folderpath == 0x00 ) {
        puts("ERROR: afs_extractEntryToFile - output_folderpath invalid.");
        return NULL;
    }

    int off = afs->header.entryinfo[id].offset;
    int size = afs->header.entryinfo[id].size;
    u8* buffer = (u8*)malloc(size);

    int folderpath_len = strlen(output_folderpath);

    // create output file path buffer and zero it out
    char* outpath = (char*)malloc(folderpath_len + AFSMETA_NAMEBUFFERSIZE + 2);
    memset(outpath, 0x00, folderpath_len + AFSMETA_NAMEBUFFERSIZE + 2);

    // copy the location of the output file path
    strncpy(outpath, output_folderpath, folderpath_len);

    // at this point, the given argument might be "/path/to/dir/"
    // or it might be "path/to/dir" (notice the missing slash at the end)
    // we need to ensure both args lead to the same result
    if(strrchr(outpath, '\\') > strrchr(outpath, '/')) {
        if(outpath[folderpath_len-1] != '\\') {
            outpath[folderpath_len] = '\\';
        }
    }
    else {
        if(outpath[folderpath_len-1] != '/') {
            outpath[folderpath_len] = '/';
        }
    }
    if(*afs->meta[id].filename != 0x00) {
        strncat(outpath, afs->meta[id].filename, AFSMETA_NAMEBUFFERSIZE);
    }
    else {
        char temp[AFSMETA_NAMEBUFFERSIZE];
        snprintf(temp, AFSMETA_NAMEBUFFERSIZE, "blank_%d", id);
        strncat(outpath, temp, AFSMETA_NAMEBUFFERSIZE);
    }

    int outpathLen = strlen(outpath);
    FILE* outfile = fopen(outpath, "wb");

    if(outfile == NULL) {
        puts("ERROR: afs_extractEntryToFile - File pointer failed to create.");
        printf("outfile path: %s\n", outpath);
        return NULL;
    }

    fseek(afs->fstream, off, SEEK_SET);
    fread(buffer, 1, size, afs->fstream);
    fwrite(buffer, 1, size, outfile);

    fclose(outfile);
    free(buffer);
    return outpath;
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

    // if windows
    if(strrchr(folderpath, '\\') > strrchr(folderpath, '/')) {
        if(output_folderpath[pathlen-1] != '\\') {
            folderpath[pathlen++] = '\\';
        }
    }
    // if linux
    else {
        if(output_folderpath[pathlen-1] != '/') {
            folderpath[pathlen++] = '/';
        }
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

int _afs_replaceEntry_noResize(Afs* afs, int id, u8* data, int data_size) {
    if(afs == NULL || afs->fstream == NULL) {
        return 1;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        return 2;
    }
    if(data == NULL || data_size <= 0) {
        return 3;
    }

    int reservedSpace = afs->header.entryinfo[id+1].offset - afs->header.entryinfo[id].offset;

    u8* newData = (u8*)malloc(reservedSpace);
    memset(newData, 0x00, reservedSpace);
    memcpy(newData, data, data_size);

    fseek(afs->fstream, afs->header.entryinfo[id].offset, SEEK_SET);
    fwrite(newData, 1, reservedSpace, afs->fstream);
    int entryinfoOffset = 8 + (sizeof(AfsEntryInfo) * id); // 8 = sizeof(identifier) + sizeof(entrycount)
    fseek(afs->fstream, entryinfoOffset, SEEK_SET);
    afs->header.entryinfo[id].size = data_size;
    fwrite(afs->header.entryinfo + id, sizeof(AfsEntryInfo), 1, afs->fstream);

    afs->meta[id].filesize = data_size;
    fseek(afs->fstream, afs->header.entryinfo[afs->header.entrycount].offset + sizeof(AfsEntryMetadata) * id, SEEK_SET);
    fwrite(afs->meta + id, sizeof(AfsEntryMetadata), 1, afs->fstream);

    fseek(afs->fstream, 0, SEEK_SET);
    free(newData);
    return 0;
}

int _afs_calcReservedSpace(int new_size) {
if(AFS_RESERVEDSPACEBUFFER % 0x10 != 0) {
        return -3;
    }

    int newReservedSpace = (new_size / AFS_RESERVEDSPACEBUFFER + 1) * AFS_RESERVEDSPACEBUFFER;

    return newReservedSpace;
}


int _afs_resizeEntrySpace(Afs* afs, int id, int new_size) {
    if(afs == NULL || afs->fstream == NULL) {
        return 1;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        return 2;
    }

    int oldSize = afs->header.entryinfo[id].size;
    afs->header.entryinfo[id].size = new_size;

    int newReservedSpace = _afs_calcReservedSpace(new_size);
    if(newReservedSpace < 0) {
        return -newReservedSpace;
    }
    u8* buffer = (u8*)malloc(newReservedSpace);
    memset(buffer, 0x00, newReservedSpace);

    // This variable is the starting offset for the back half of the AFS
    int oldOffsetNextEntry = afs->header.entryinfo[id+1].offset;

    // This loop updates each element in the entryinfo array with their new offsets.
    int curOffset = afs->header.entryinfo[id].offset + newReservedSpace;
    for(int i=id + 1; i < afs->header.entrycount; i++) {
        int reserved = afs->header.entryinfo[i+1].offset - afs->header.entryinfo[i].offset;
        afs->header.entryinfo[i].offset = curOffset;
        curOffset += reserved;
    }
    // Metadata
    afs->header.entryinfo[afs->header.entrycount].offset = curOffset;

    // Consider the AFS as one long line of data.
    // This is splitting the AFS exactly where the entry is,
    // Creating a Front part and a back part.

    // This is the size of that back part
    fseek(afs->fstream, oldOffsetNextEntry, SEEK_SET);
    int backSize = ftell(afs->fstream);
    fseek(afs->fstream, 0, SEEK_END);
    backSize = ftell(afs->fstream) - backSize;

    // This here reads the back part into a buffer
    u8* back = (u8*)malloc(backSize);
    fseek(afs->fstream, oldOffsetNextEntry, SEEK_SET);;
    fread(back, 1, backSize, afs->fstream);

    // This here writes the new space to the file
    fseek(afs->fstream, afs->header.entryinfo[id].offset, SEEK_SET);
    fwrite(buffer, 1, newReservedSpace, afs->fstream);
    // This here writes the back part to the file
    fwrite(back, 1, backSize, afs->fstream);

    // This here writes the new entry info into the header.
    fseek(afs->fstream, 8, SEEK_SET);
    fwrite(afs->header.entryinfo, sizeof(AfsEntryInfo), afs->header.entrycount + 1, afs->fstream);

    free(back);
    free(buffer);
    return 0;
}


int afs_replaceEntry(Afs* afs, int id, u8* data, int data_size) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_replaceEntry - Invalid AFS File.");
        return 1;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        puts("ERROR: afs_replaceEntry - Entry ID out of range.");
        printf("Entry ID: %d, AFS Entry Count: %d\n", id, afs->header.entrycount);
        return 2;
    }
    if(data == NULL || data_size <= 0) {
        puts("ERROR: afs_replaceEntry - Given data is invalid (NULL or zero size).");
        printf("data: 0x%08x\tdata_size: %d", *(unsigned int*)&data, data_size);
        return 3;
    }

    int reservedSpace = afs->header.entryinfo[id+1].offset - afs->header.entryinfo[id].offset;

    if(data_size >= reservedSpace) {
        _afs_resizeEntrySpace(afs, id, data_size);
    }
    _afs_replaceEntry_noResize(afs, id, data, data_size);

    return 0;
}

int afs_replaceEntriesFromFiles(Afs* afs, int* entries, char** filepaths, int amount_entries) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_replaceEntriesFromFiles - Invalid AFS File.");
        return 1;
    }
    if(entries == NULL || filepaths == NULL) {
        puts("ERROR: afs_replaceEntriesFromFiles - Invalid array args.");
        return 2;
    }
    if(amount_entries <= 0) {
        puts("ERROR: afs_replaceEntriesFromFiles - Invalid replaced entry count.");
        return 3;
    }

    // Store the old entryinfo array
    AfsEntryInfo* oldEntries = (AfsEntryInfo*)malloc(sizeof(AfsEntryInfo) * (afs->header.entrycount + 1));
    memcpy(oldEntries, afs->header.entryinfo, sizeof(AfsEntryInfo) * (afs->header.entrycount + 1));

    // Store the files that are meant to replace the entries
    u8* fileData[amount_entries];
    int fileSizes[amount_entries];

    bool allEntriesSkipped = true;
    for(int i=0;i<amount_entries;i++) {
        // If the file is marked "skip", we skip it
        if(entries[i] == -1) {
            continue;
        }
        allEntriesSkipped = false;
        FILE* curFile = fopen(filepaths[i], "rb");
        if(curFile == NULL) {
            puts("ERROR: afs_replaceEntriesFromFiles - a filepath isn't accessible or doesn't exist!");
            printf("File path #%d: %s\n", i, filepaths[i]);
            return 2;
        }
        int size = ftell(curFile);
        fseek(curFile, 0, SEEK_END);
        size = ftell(curFile) - size;
        fileSizes[i] = size;

        fileData[i] = (u8*)malloc(size);

        fseek(curFile, 0, SEEK_SET);
        fread(fileData[i], 1, size, curFile);

        char* filename = NULL;
        char* win_fname = strrchr(filepaths[i], '\\');
        char* unix_fname = strrchr(filepaths[i], '/');
        if(win_fname > unix_fname) filename = win_fname + 1;
        else if(unix_fname == NULL) filename = filepaths[i];
        else filename = unix_fname + 1;

        strncpy(afs->meta[entries[i]].filename, filename, AFSMETA_NAMEBUFFERSIZE);
        afs->meta[entries[i]].filesize = size;

        fclose(curFile);
    }
    if(allEntriesSkipped) {
        puts("ERROR: afs_replaceEntriesFromFiles - All entries were skipped.");
        return 2;
    }

    // Change the AFS Header info
    int curOffset = oldEntries[0].offset;
    for(int i=0; i < afs->header.entrycount; i++) {
        bool isImported = false;
        afs->header.entryinfo[i].offset = curOffset;

        for(int j=0;j<amount_entries;j++) {
            if(entries[j] == i) {
                isImported = true;
                afs->header.entryinfo[i].size = fileSizes[j];
                int reservedSpace = _afs_calcReservedSpace(fileSizes[j]);
                if(reservedSpace > 0) curOffset += reservedSpace;
                break;
            }
        }
        if(isImported) continue;

        afs->header.entryinfo[i].size = oldEntries[i].size;
        int reservedSpace = oldEntries[i+1].offset - oldEntries[i].offset;
        curOffset += reservedSpace;
    }
    // Metadata
    afs->header.entryinfo[afs->header.entrycount].offset = curOffset;

    // Create output buffer
    int dataSectionSize_new = curOffset - oldEntries[0].offset;
    int dataSectionOffset = afs->header.entryinfo[0].offset;
    u8* buffer = (u8*)malloc(dataSectionSize_new);
    memset(buffer, 0x00, dataSectionSize_new);
    // Insert data
    for(int i=0; i < afs->header.entrycount; i++) {
        bool isImported = false;
        for(int j=0;j<amount_entries;j++) {
            if(entries[j] == i) {
                isImported = true;
                // If this is an entry that should be replaced, we take its file data and insert it
                memcpy(buffer + afs->header.entryinfo[i].offset - dataSectionOffset, fileData[j], fileSizes[j]);
                break;
            }
        }
        if(isImported) continue;

        // If this is a regular entry, we fread it from the AFS and insert it into the buffer that way
        fseek(afs->fstream, oldEntries[i].offset, SEEK_SET);
        fread(buffer + afs->header.entryinfo[i].offset - dataSectionOffset, 1, afs->header.entryinfo[i].size, afs->fstream);
    }

    // Update Metadata
    for(int i=0;i<amount_entries;i++) {
        AfsEntryMetadata* meta = afs->meta + entries[i];
        // Filename
        char* unix_filename = strrchr(filepaths[i], '/') + 1;
        char* win_filename = strrchr(filepaths[i], '\\') + 1;
        if(win_filename > unix_filename) {
            strncpy(meta->filename, win_filename, AFSMETA_NAMEBUFFERSIZE);
        }
        else {
            strncpy(meta->filename, unix_filename, AFSMETA_NAMEBUFFERSIZE);
        }
        // Last Modified Date
        time_t current_time = time(NULL);
        struct tm tm = *localtime(&current_time);
        meta->lastModified.year = tm.tm_year + 1900;
        meta->lastModified.month = tm.tm_mon + 1;
        meta->lastModified.day = tm.tm_mday;
        meta->lastModified.hours = tm.tm_hour;
        meta->lastModified.minutes = tm.tm_min;
        meta->lastModified.seconds = tm.tm_sec;
        // File Size
        meta->filesize = fileSizes[i];
    }

    // Write the new entryinfo to the AFS file
    fseek(afs->fstream, 8, SEEK_SET);
    fwrite(afs->header.entryinfo, sizeof(AfsEntryInfo), afs->header.entrycount+1, afs->fstream);

    // The Big Write
    fseek(afs->fstream, afs->header.entryinfo[0].offset, SEEK_SET);
    fwrite(buffer, 1, dataSectionSize_new, afs->fstream);

    // Write metadata to the AFS file
    fseek(afs->fstream, afs->header.entryinfo[afs->header.entrycount].offset, SEEK_SET);
    fwrite(afs->meta, sizeof(AfsEntryMetadata), afs->header.entrycount, afs->fstream);

    free(buffer);
    for(int i=0;i<amount_entries;i++) {
        if(entries[i] != -1) {
            free(fileData[i]);
        }
    }
    free(oldEntries);
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

AfsEntryMetadata afs_getEntryMetadata(Afs* afs, int id) {
    AfsEntryMetadata out = {};

    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_getEntryMetadata - Invalid AFS File.");
        return out;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        puts("ERROR: afs_getEntryMetadata - Entry ID out of range.");
        printf("Entry ID: %d, AFS Entry Count: %d\n", id, afs->header.entrycount);
        return out;
    }

    memcpy(&out, &(afs->meta[id]), sizeof(AfsEntryMetadata));

    return out;
}

int afs_setEntryMetadata(Afs* afs, int id, AfsEntryMetadata new_meta, bool permanent) {
    if(afs == NULL || afs->fstream == NULL) {
        puts("ERROR: afs_setEntryMetadata - Invalid AFS File.");
        return 1;
    }
    if(id < 0 || id >= afs->header.entrycount) {
        puts("ERROR: afs_setEntryMetadata - Entry ID out of range.");
        printf("Entry ID: %d, AFS Entry Count: %d\n", id, afs->header.entrycount);
        return 2;
    }
    memcpy(&(afs->meta[id]), &new_meta, sizeof(AfsEntryMetadata));

    if(permanent) {
        fseek(afs->fstream, afs->header.entryinfo[afs->header.entrycount].offset + (id * sizeof(AfsEntryMetadata)), SEEK_SET);
        fwrite(&new_meta, sizeof(AfsEntryMetadata), 1, afs->fstream);
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
    char* out = (char*)calloc(1, 32);
    sprintf(out, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d", t.day, t.month, t.year, t.hours, t.minutes, t.seconds);
    return out;
}
