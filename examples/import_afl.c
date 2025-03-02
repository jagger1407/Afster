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
    puts("import_afl - Import an AFL File into an AFS.\n");
    puts("arg1 = A path to an AFS File");
    puts("arg2 = A path to an AFL File");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we open an AFL file list and import it into the metadata section of the AFS file.
 *
 * We take 2 arguments:
 * arg1 = A path to an AFS File
 * arg2 = A path to an AFL File 
*/
int main(int argc, char** argv) {
    // Checking if all arguments are present
    if(argc < 2) {
        puts("ERROR: main - No AFS file specified.");
        printHelp();
        return 1;
    }
    else if(argc < 3) {
        puts("ERROR: main - No AFL option specified.");
        printHelp();
        return 2;
    }
    // Checking whether the given path points to an AFS File
    int len = strlen(argv[1]);
    char afspath[len];
    strcpy(afspath, argv[1]);
    if(strcmp(strlwr(afspath) + len - 4, ".afs") != 0) {
        puts("ERROR: main - arg1 is not an AFS File.");
        printHelp();
        return 1;
    }
    // If they are, we create the AFS handle using afs_open()...
    Afs* afs = afs_open(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 1;
    }
    // ...and the AFL handle using afl_open().
    Afl* afl = afl_open(argv[2]);
    if(afl == NULL) {
        puts("ERROR: main - AFL file couldn't be created.");
        return 2;
    }

    // Now with both of the handles created, all that's left is to combine the 2.

    // Unlike extract_afs.c, here, we pass true to the function
    // in order to write the names into the AFS file permanently.
    afl_importAfl(afl, afs, true);

    // And of course, we free both of these handles after usage.
    afl_free(afl);
    afs_free(afs);
    return 0;
}
