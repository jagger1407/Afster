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

Afl_t* openAfl(const char* aflPath) {
    if(aflPath == nullptr || *aflPath == '\0') {
        return NULL;
    }
    FILE* fp = fopen(aflPath, "rb");

    if(fp == NULL) {
        puts("ERROR Afl::openAfl - AFL Filepath invalid.");
        printf("Filepath: %s\n", aflPath);
        return NULL;
    }
    Afl_t* afl = (Afl_t*)malloc(sizeof(Afl_t));
    afl->fstream = fp;

    fread(&afl->head, 0x10, 1, afl->fstream);

    afl->filenames = (char*)malloc(AFL_NAMEBUFFERSIZE * afl->head.filecount);
    fread(afl->filenames, AFL_NAMEBUFFERSIZE, afl->head.filecount, fp);

    fseek(fp, 0, SEEK_SET);

    return afl;
}

char* getAflName(Afl_t* afl, int id) {
    return AFL_NAME(afl, id);
}

void freeAfl(Afl_t* afl) {
    fclose(afl->fstream);
    free(afl->filenames);
    free(afl);
}

#endif // AFL_H_INCLUDED
