#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../afs.h"

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
        return 1;
    }
    else if(argc < 3) {
        puts("ERROR: main - No AFL option specified.");
        return 2;
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
    afs_importAfl(afs, afl, true);

    // And of course, we free both of these handles after usage.
    afl_free(afl);
    afs_free(afs);
    return 0;
}
