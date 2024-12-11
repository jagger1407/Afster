#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "afs.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        puts("ERROR: main - No AFS file specified.");
        return 1;
    }

    Afs_t* afs = openAfs(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 2;
    }

    if(*argv[2] != 'n') {
        Afl_t* afl = openAfl(argv[2]);
        if(afl == NULL) {
            puts("ERROR: main - AFL file couldn't be created.");
            return 3;
        }
        importAfl(afs, afl, false);

        freeAfl(afl);
    }

    if(argv[3] == NULL || *argv[3] == 0x00) {
        puts("ERROR: main - Output folder doesn't exist or is invalid");
        printf("Output folder: %s\n", argv[3]);
        return 4;
    }

    extractWholeAfs(afs, argv[3]);

    freeAfs(afs);
    return 0;
}
