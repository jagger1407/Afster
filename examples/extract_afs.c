#include <stdio.h>
#include <stdlib.h>
#include "../afs.h"

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we extract the entire AFS File into a folder.
 *
 * This program takes in 3 arguments:
 * arg1 = A path to an AFS file
 * arg2 = Either a path to an AFL file or the letter 'n' in case no AFL is wanted
 * arg3 = A path to an output folder where all entries are to be stored
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
    else if(argc < 4) {
        puts("ERROR: main - No output path specified.");
        return 3;
    }

    // Now that we know all arguments are there,
    // we can create an AFS Handle with the first argument
    // using the afs_open() function.
    Afs* afs = afs_open(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 1;
    }

    if(*argv[2] != 'n') {
        // In case it is wanted, we create an AFL Handle using afl_open()
        Afl* afl = afl_open(argv[2]);
        if(afl == NULL) {
            puts("ERROR: main - AFL file couldn't be created.");
            return 2;
        }
        // Then we import the AFL list into the metadata section of the AFS,
        // but since we pass false here, the actual AFS File isn't updated,
        // it is only imported in-memory.
        afs_importAfl(afs, afl, false);

        // Lastly, to ensure there is no memory leak,
        // we free the AFL since we aren't using it anymore.
        afl_free(afl);
    }

    // Now we check the validity of the output folder.
    if(argv[3] == NULL || *argv[3] == 0x00) {
        puts("ERROR: main - Output folder doesn't exist or is invalid");
        printf("Output folder: %s\n", argv[3]);
        afs_free(afs);
        return 3;
    }

    // And this one function call here fully extracts the entire AFS File.
    afs_extractFull(afs, argv[3]);

    // Lastly we make sure that no memory leaks occur by freeing all AFS related memory.
    afs_free(afs);
    return 0;
}
