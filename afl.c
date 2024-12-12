#include "afl.h"

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
