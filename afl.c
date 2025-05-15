#include "afl.h"

Afl* afl_open(const char* aflPath) {
    if(aflPath == nullptr || *aflPath == '\0') {
        return NULL;
    }
    FILE* fp = fopen(aflPath, "rb+");

    if(fp == NULL) {
        puts("ERROR: afl_open - AFL Filepath invalid.");
        printf("Filepath: %s\n", aflPath);
        return NULL;
    }
    Afl* afl = (Afl*)malloc(sizeof(Afl));
    afl->fstream = fp;

    fread(&afl->head, 0x10, 1, afl->fstream);

    afl->entrynames = (char*)malloc(AFL_NAMEBUFFERSIZE * afl->head.filecount);
    fread(afl->entrynames, AFL_NAMEBUFFERSIZE, afl->head.filecount, fp);

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

int afl_rename(Afl* afl, int id, const char* newName) {
    if(afl == NULL) {
        puts("ERROR: afl_rename - afl null.");
        return 1;
    }
    if(afl->fstream == NULL) {
        puts("ERROR: afl_rename - afl exists, but afs->fstream doesn't.");
        return 1;
    }
    if(id < 0 || id > afl->head.filecount) {
        puts("ERROR: afl_rename - Entry ID out of range.");
        return 2;
    }

    strncpy(_AFL_NAME(afl, id), newName, AFL_NAMEBUFFERSIZE);

    return 0;
}

void afl_free(Afl* afl) {
    if(afl == NULL) {
        puts("WARNING: afl_free - afl pointer already freed. Returning.");
        return;
    }
    fclose(afl->fstream);
    free(afl->entrynames);
    free(afl);
    afl = NULL;
}

int afl_save(Afl* afl) {
    if(afl == NULL) {
        puts("ERROR: afl_save - afl null.");
        return 1;
    }
    if(afl->fstream == NULL) {
        puts("ERROR: afl_save - fstream null.");
        return 1;
    }
    fseek(afl->fstream, 0, SEEK_SET);
    fwrite(&afl->head, sizeof(AflHeader), 1, afl->fstream);
    fwrite(afl->entrynames, AFL_NAMEBUFFERSIZE, afl->head.filecount, afl->fstream);

    return 0;
}

int afl_saveNew(Afl* afl, const char* filepath) {
    if(afl == NULL) {
        puts("ERROR: afl_save - afl null.");
        return 1;
    }
    FILE* outfile = fopen(filepath, "wb+");
    if(outfile == NULL) {
        puts("ERROR: afl_save - filepath can't be written to.");
        return 2;
    }
    fseek(afl->fstream, 0, SEEK_SET);
    fwrite(&afl->head, sizeof(AflHeader), 1, outfile);
    fwrite(afl->entrynames, AFL_NAMEBUFFERSIZE, afl->head.filecount, outfile);

    fclose(outfile);
    return 0;
}

int afl_importAfl(Afl* afl, Afs* afs, bool permament) {
    if(afs == NULL) {
        puts("ERROR: afl_importAfl - afs null.");
        return 1;
    }
    if(afs->fstream == NULL) {
        puts("ERROR: afl_importAfl - afs exists, but afs->fstream doesn't.");
        return 1;
    }
    if(afl == NULL) {
        puts("ERROR: afl_importAfl - afl null.");
        return 2;
    }

    if(afl->head.filecount != afs->header.entrycount) {
        puts("WARNING: afl_importAfl - AFS file count and AFL file count are mismatched.");
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
