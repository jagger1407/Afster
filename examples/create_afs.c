#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../afs.h"
#include "../afl.h"

char* strlwr(char* str) {
    char* curChar = str;
    while(*curChar) {
        if(*curChar >= 'A' && *curChar <= 'Z')
            *curChar += 0x20;
        curChar++;
    }
    return str;
}

/** Prints a help text explaining how to use this program
 */
void printHelp() {
    puts("create_afs - Creates an AFS from a given folder.\n");
    puts("arg1 = A path to a folder");
    puts("arg2 = A path to an AFL file");
    puts("arg3 = A path to an output AFS file");
}

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    char path[PATH_MAX];
    Timestamp lm;
    u64 size;
    u64 paddedSize;
} AfsSubfile;

u32 getPaddedSize(int filesize) {
    if(filesize % AFS_RESERVEDSPACEBUFFER != 0) {
        return filesize + (AFS_RESERVEDSPACEBUFFER - (filesize % AFS_RESERVEDSPACEBUFFER));
    }
    else {
        return filesize;
    }
}

#ifdef _WIN32
#include <windows.h>

Timestamp getLastModified(FILETIME* ft) {
    Timestamp afstime;
    SYSTEMTIME stUTC, stlocal;
    FileTimeToSystemTime(ft, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stlocal);
    afstime.year = stlocal.wYear;
    afstime.month = stlocal.wMonth;
    afstime.day = stlocal.wDay;
    afstime.hours = stlocal.wHour;
    afstime.minutes = stlocal.wMinute;
    afstime.seconds = stlocal.wSecond;

    return afstime;
}

AfsSubfile* getFiles(char* folderpath, int* filecount, Afl* afl) {
    char* rpath = (char*)calloc(PATH_MAX, 1);
    GetFullPathNameA(folderpath, PATH_MAX, rpath, NULL);
    WIN32_FIND_DATAA fd;
    char* dir = (char*)calloc(PATH_MAX, 1);
    snprintf(dir, PATH_MAX, "%s\\*", rpath);
    HANDLE hfd = FindFirstFileA(dir, &fd);
    do {
        if(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            (*filecount)++;
        }
    } while(FindNextFileA(hfd, &fd) != 0);
    FindClose(hfd);

    if(*filecount == 0) {
        printf("ERROR: getFiles - No files found in '%s'.\n", folderpath);
        return NULL;
    }

    AfsSubfile* filelist = (AfsSubfile*)malloc(*filecount * sizeof(AfsSubfile));
    int entrycount = afl_getEntrycount(afl);

    hfd = FindFirstFileA(dir, &fd);
    do {
        if(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        
        AfsSubfile* cur = NULL;
        for(int i=0;i<entrycount;i++) {
            if(strcmp(fd.cFileName, afl_getName(afl, i)) == 0) {
                cur = &filelist[i];
            }
        }
        if(cur == NULL) continue;

        if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            snprintf(cur->path, PATH_MAX, "%s\\%s", rpath, fd.cFileName);
            cur->size = ((u64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            cur->paddedSize = getPaddedSize(cur->size);
            cur->lm = getLastModified(&fd.ftLastAccessTime);
        }
    } while(FindNextFileA(hfd, &fd) != 0);

    FindClose(hfd);
    free(rpath);
    return filelist;
}

#elif __linux__
#include <dirent.h>
#include <fcntl.h> 
#include <sys/stat.h>

Timestamp getLastModified(struct stat* st) {
    struct tm* time = localtime(&st->st_mtime);
    
    Timestamp afstime;
    afstime.year = time->tm_year + 1900;
    afstime.month = time->tm_mon + 1;
    afstime.day = time->tm_mday;
    afstime.hours = time->tm_hour;
    afstime.minutes = time->tm_min;
    afstime.seconds = time->tm_sec;

    return afstime;
}

AfsSubfile* getFiles(char* folderpath, int* filecount, Afl* afl) {
    char* rpath = (char*)calloc(PATH_MAX, 1);
    realpath(folderpath, rpath);
    
    DIR* dir = opendir(rpath);
    struct dirent* cur;
    char fpath[PATH_MAX];
    while(cur = readdir(dir), cur != NULL) {
        if(strcmp(cur->d_name, ".") == 0 || strcmp(cur->d_name, "..") == 0) continue;
        snprintf(fpath, PATH_MAX, "%s/%s", rpath, cur->d_name);

        struct stat curStat;
        lstat(fpath, &curStat);
        if(S_ISREG(curStat.st_mode)) {
            (*filecount)++;
        }
    }
    rewinddir(dir);

    if(*filecount == 0) {
        printf("ERROR: getFiles - No files found in '%s'.\n", folderpath);
        return NULL;
    }

    int entrycount = afl_getEntrycount(afl);

    AfsSubfile* filelist = (AfsSubfile*)malloc(*filecount * sizeof(AfsSubfile));
    while(cur = readdir(dir), cur != NULL) {
        if(strcmp(cur->d_name, ".") == 0 || strcmp(cur->d_name, "..") == 0) continue;
        
        AfsSubfile* cursub = NULL;
        for(int i=0;i<entrycount;i++) {
            if(strcmp(cur->d_name, afl_getName(afl, i)) == 0) {
                cursub = &filelist[i];
            }
        }
        if(cursub == NULL) continue;
        snprintf(cursub->path, PATH_MAX, "%s/%s", rpath, cur->d_name);

        struct stat curStat;
        lstat(cursub->path, &curStat);
        if(S_ISREG(curStat.st_mode)) {
            cursub->lm = getLastModified(&curStat);
            cursub->size = curStat.st_size;
            cursub->paddedSize = getPaddedSize(cursub->size);
        }
    }

    closedir(dir);
    free(rpath);
    return filelist;
}

#endif

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we take as an argument a path to an input folder,
 * an AFL File List, and an output path, and we use these to create an AFS file from scratch.
*/
int main(int argc, char** argv) {
    // We first check whether the AFS argument even exists.
    if(argc < 2) {
        puts("ERROR: main - No input folder specified.");
        printHelp();
        return 1;
    }
    if(argc < 3) {
        puts("ERROR: main - No AFL File specified.");
        printHelp();
        return 2;
    }
    if(argc < 4) {
        puts("ERROR: main - No output path specified.");
        printHelp();
        return 3;
    }
    // We then check whether this argument links to an AFL file
    // We take the length of the given filepath
    int len = strlen(argv[2]);
    char aflpath[len];
    strcpy(aflpath, argv[2]);
    // strlwr turns the string into all lowercase,
    // then we strcmp the last 4 letters to be ".afl"
    if(strcmp(strlwr(aflpath) + len - 4, ".afl") != 0) {
        puts("ERROR: main - arg2 is not an AFL File.");
        printHelp();
        return 2;
    }

    puts("Reading AFL...");
    Afl* afl = afl_open(argv[2]);
    int entrycount = afl_getEntrycount(afl);

    int metasize = entrycount * sizeof(AfsEntryMetadata);
    AfsEntryMetadata* metadata = (AfsEntryMetadata*)malloc(metasize);

    int infosize = (entrycount+1) * sizeof(AfsEntryInfo);
    AfsEntryInfo* entinfo = (AfsEntryInfo*)malloc(infosize);

    puts("Getting files...");
    int fcount = 0;
    AfsSubfile* files = getFiles(argv[1], &fcount, afl);
    printf("%d files found.\n", fcount);

    if(files == NULL) {
        return 1;
    }

    u32* curval = (u32*)entinfo;
    puts("Calculating file offsets...");
    u32 curOffset = getPaddedSize(infosize + 8);
    for(int i=0;i<entrycount;i++) {
        char* entry = afl_getName(afl, i);
        AfsSubfile* file = &files[i];
        entinfo[i].offset = curOffset;
        entinfo[i].size = file->size;
        curOffset += file->paddedSize;

        strncpy(metadata[i].filename, entry, 32);
        if(i != 0) {
            metadata[i].filesize = *curval;
            curval++;
        }
        metadata[i].lastModified = file->lm;
    }
    entinfo[entrycount].offset = curOffset;
    entinfo[entrycount].size = metasize;
    metadata[0].filesize = entrycount;

    puts("Creating AFS...");
    Afs* afs = (Afs*)malloc(sizeof(Afs));
    strncpy(afs->header.identifier, "AFS", 4);
    afs->fstream = fopen(argv[3], "wb");
    afs->header.entrycount = entrycount;
    afs->header.entryinfo = entinfo;
    afs->meta = metadata;
    
    fwrite(&afs->header, 8, 1, afs->fstream);
    fwrite(afs->header.entryinfo, sizeof(AfsEntryInfo), entrycount+1, afs->fstream);
    fseek(afs->fstream, afs->header.entryinfo[0].offset, SEEK_SET);
    
    for(int i=0;i<entrycount;i++) {
        fseek(afs->fstream, afs->header.entryinfo[i].offset, SEEK_SET);
        AfsSubfile* file = &files[i];
        u8* buf = (u8*)calloc(file->paddedSize, 1);
        FILE* fp = fopen(file->path, "rb");
        if(fp == NULL) {
            printf("'%s' not found. Skipping.\n", afs->meta[i].filename);
            continue;
        }      
        fread(buf, file->size, 1, fp);
        fwrite(buf, file->paddedSize, 1, afs->fstream);
        fclose(fp);
        free(buf);
    }
    u8* buf = (u8*)calloc(getPaddedSize(metasize), 1);
    memcpy(buf, afs->meta, metasize);
    fwrite(buf, getPaddedSize(metasize), 1, afs->fstream);
    printf("'%s' created.\n", argv[3]);

    free(buf);
    afs_free(afs);
    afl_free(afl);
    free(files);
    return 0;
}
