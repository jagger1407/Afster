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

int afl_importAfl(Afl* afl, Afs* afs, bool permament) {
    if(afs == NULL) {
        puts("ERROR: afs_importAfl - afs null.");
        return 1;
    }
    if(afs->fstream == NULL) {
        puts("ERROR: afs_importAfl - afs exists, but afs->fstream doesn't.");
        return 1;
    }
    if(afl == NULL) {
        puts("ERROR: afs_importAfl - afl null.");
        return 2;
    }

    if(afl->head.filecount != afs->header.entrycount) {
        puts("WARNING: afs_importAfl - AFS file count and AFL file count are mismatched.");
        printf("AFS file count: %d\nAFL file count: %d\n", afs->header.entrycount, afl->head.filecount);
    }

    for(int i=0;i<afs->header.entrycount;i++) {
        memcpy(afs->meta[i].filename, afl_getName(afl, i), AFSMETA_NAMEBUFFERSIZE);
    }
    if(permament) {
        AfsEntryInfo metaInfo = afs->header.entryinfo[afs->header.entrycount];
        fseek(afs->fstream, metaInfo.offset, SEEK_SET);
        fwrite(afs->meta, metaInfo.size, 1, afs->fstream);
    }
    return 0;
}
