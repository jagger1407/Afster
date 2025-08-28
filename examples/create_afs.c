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

#ifdef _WIN32
#include <windows.h>

char** getFiles(char* folderpath, int* filecount) {
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

    char** filelist = (char**)malloc(*filecount * sizeof(char*));
    int ifl = 0;
    char* fpath = (char*)calloc(PATH_MAX, 1);
    hfd = FindFirstFileA(dir, &fd);
    do {
        if(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        snprintf(fpath, PATH_MAX, "%s\\%s", rpath, fd.cFileName);
        if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            char* curfile = (char*)calloc(PATH_MAX, 1);
            strncpy(curfile, fpath, PATH_MAX);
            filelist[ifl] = curfile;
            ifl++;
        }
    } while(FindNextFileA(hfd, &fd) != 0);

    FindClose(hfd);
    free(fpath);
    free(rpath);
    return filelist;
}

Timestamp getLastModified(char* filepath) {
    Timestamp afstime;
    memset(&afstime, 0x00, sizeof(Timestamp));
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if(!GetFileAttributesExA(filepath, GetFileExInfoStandard, &attr)) return afstime;
    FILETIME t = attr.ftLastWriteTime;
    SYSTEMTIME stUTC, stlocal;
    FileTimeToSystemTime(&t, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stlocal);
    afstime.year = stlocal.wYear;
    afstime.month = stlocal.wMonth;
    afstime.day = stlocal.wDay;
    afstime.hours = stlocal.wHour;
    afstime.minutes = stlocal.wMinute;
    afstime.seconds = stlocal.wSecond;

    return afstime;
}

int indexOf(char** arr, int count, char* searchstr) {
    for(int i=0;i<count;i++) {
        char* cur = strrchr(arr[i], '\\') + 1;
        if(strcmp(cur, searchstr) == 0) {
            return i;
        }
    }
    return -1;
}

#elif __linux__
#include <dirent.h>
#include <fcntl.h> 
#include <sys/stat.h>

char** getFiles(char* folderpath, int* filecount) {
    char* rpath = (char*)calloc(PATH_MAX, 1);
    realpath(folderpath, rpath);
    
    DIR* dir = opendir(rpath);
    struct dirent* cur;
    char* fpath = (char*)calloc(PATH_MAX, 1);
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

    char** filelist = (char**)malloc(*filecount * sizeof(char*));
    int ifl = 0;
    while(cur = readdir(dir), cur != NULL) {
        if(strcmp(cur->d_name, ".") == 0 || strcmp(cur->d_name, "..") == 0) continue;
        snprintf(fpath, PATH_MAX, "%s/%s", rpath, cur->d_name);

        struct stat curStat;
        lstat(fpath, &curStat);
        if(S_ISREG(curStat.st_mode)) {
            char* curfile = (char*)calloc(PATH_MAX, 1);
            strncpy(curfile, fpath, PATH_MAX);
            filelist[ifl] = curfile;
            ifl++;
        }
    }

    closedir(dir);
    free(fpath);
    free(rpath);
    return filelist;
}

Timestamp getLastModified(char* filepath) {
    struct stat st;
    stat(filepath, &st);
    struct tm* time = localtime(&st.st_mtime);
    
    Timestamp afstime;
    afstime.year = time->tm_year + 1900;
    afstime.month = time->tm_mon + 1;
    afstime.day = time->tm_mday;
    afstime.hours = time->tm_hour;
    afstime.minutes = time->tm_min;
    afstime.seconds = time->tm_sec;

    return afstime;
}

int indexOf(char** arr, int count, char* searchstr) {
    for(int i=0;i<count;i++) {
        char* cur = strrchr(arr[i], '/') + 1;
        if(strcmp(cur, searchstr) == 0) {
            return i;
        }
    } 
}

#endif

u64 getFilesize(char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    u64 size = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp) - size;
    fclose(fp);
    return size;
}

u32 getPaddedSize(int filesize) {
    if(filesize % AFS_RESERVEDSPACEBUFFER != 0) {
        return filesize + (AFS_RESERVEDSPACEBUFFER - (filesize % AFS_RESERVEDSPACEBUFFER));
    }
    else {
        return filesize;
    }
}

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

    puts("Getting files...");
    int fcount = 0;
    char** files = getFiles(argv[1], &fcount);
    printf("%d files found.\n", fcount);

    puts("Reading AFL...");
    Afl* afl = afl_open(argv[2]);
    int entrycount = afl_getEntrycount(afl);

    int metasize = entrycount * sizeof(AfsEntryMetadata);
    AfsEntryMetadata* metadata = (AfsEntryMetadata*)malloc(metasize);

    int infosize = (entrycount+1) * sizeof(AfsEntryInfo);
    AfsEntryInfo* entinfo = (AfsEntryInfo*)malloc(infosize);

    u32* curval = (u32*)entinfo;
    puts("Calculating file offsets...");
    u32 curOffset = getPaddedSize(infosize + 8);
    for(int i=0;i<entrycount;i++) {
        char* entry = afl_getName(afl, i);
        char* path = files[indexOf(files, fcount, entry)];
        entinfo[i].offset = curOffset;
        entinfo[i].size = getFilesize(path);
        curOffset += getPaddedSize(entinfo[i].size);

        strncpy(metadata[i].filename, entry, 32);
        if(i != 0) {
            metadata[i].filesize = *curval;
            curval++;
        }
        metadata[i].lastModified = getLastModified(path);
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
        char* file = files[indexOf(files, fcount, afs->meta[i].filename)];
        FILE* fp = fopen(file, "rb");
        if(fp == NULL) {
            printf("'%s' not found. Skipping.\n", afs->meta[i].filename);
            continue;
        }
        u32 bufsize = afs->header.entryinfo[i+1].offset - afs->header.entryinfo[i].offset;
        u8* buf = (u8*)calloc(bufsize, 1);
        fread(buf, afs->header.entryinfo[i].size, 1, fp);
        fwrite(buf, bufsize, 1, afs->fstream);
        free(buf);
        fclose(fp);
    }
    u8* buf = (u8*)calloc(getPaddedSize(metasize), 1);
    memcpy(buf, afs->meta, metasize);
    fwrite(buf, getPaddedSize(metasize), 1, afs->fstream);
    printf("'%s' created.\n", argv[3]);

    free(buf);
    afs_free(afs);
    afl_free(afl);
    for(int i=0;i<fcount;i++) {
        free(files[i]);
    }
    free(files);
    return 0;
}
