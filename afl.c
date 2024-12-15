#include "afl.h"

Afl* afl_open(const char* aflPath) {
    if(aflPath == nullptr || *aflPath == '\0') {
        return NULL;
    }
    FILE* fp = fopen(aflPath, "rb");

    if(fp == NULL) {
        puts("ERROR: afl_open - AFL Filepath invalid.");
        printf("Filepath: %s\n", aflPath);
        return NULL;
    }
    Afl* afl = (Afl*)malloc(sizeof(Afl));
    afl->fstream = fp;

    fread(&afl->head, 0x10, 1, afl->fstream);

    afl->filenames = (char*)malloc(AFL_NAMEBUFFERSIZE * afl->head.filecount);
    fread(afl->filenames, AFL_NAMEBUFFERSIZE, afl->head.filecount, fp);

    fseek(fp, 0, SEEK_SET);

    return afl;
}

char* afl_getName(Afl* afl, int id) {
    if(id < 0 || id >= afl->head.filecount) {
        puts("ERROR: afl_getName - Entry ID out of range.");
        printf("Entry ID: %d\tAFL Entry Count: %d\n", id, afl->head.filecount);
        return NULL;
    }
    return _AFL_NAME(afl, id);
}

void afl_free(Afl* afl) {
    if(afl == NULL) {
        puts("WARNING: afl_free - afl pointer already freed. Returning.");
        return;
    }
    fclose(afl->fstream);
    free(afl->filenames);
    free(afl);
    afl = NULL;
}
