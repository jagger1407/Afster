#include <stdio.h>
#include <stdlib.h>
#include "../afl.h"
#include "../afs.h"

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
    puts("extract - Extract all entries within an AFS.\n");
    puts("arg1 = A path to an AFS file");
    puts("arg2 = Either a path to an AFL file, or the letter 'n' in case no AFL is wanted");
    puts("arg3 = The index of a singular entry to extract, or the letter 'n' in case all files should be extracted");
    puts("arg4 = A path to an output folder where all entries are to be stored");
}

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
        printHelp();
        return 1;
    }
    else if(argc < 3) {
        puts("ERROR: main - No AFL option specified.");
        return 2;
    }
    else if(argc < 4) {
        puts("ERROR: main - No entry selection specified.");
        return 3;
    }
    else if(argc < 5) {
        puts("ERROR: main - No output folder specified.");
        return 4;
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
        afl_importAfl(afl, afs, false);

        // Lastly, to ensure there is no memory leak,
        // we free the AFL since we aren't using it anymore.
        afl_free(afl);
    }

    // Now we check the validity of the output folder.
    if(argv[4] == NULL || *argv[4] == 0x00) {
        puts("ERROR: main - Output folder doesn't exist or is invalid");
        printf("Output folder: %s\n", argv[3]);
        afs_free(afs);
        return 3;
    }

    // We check which option the user has given us
    if(*argv[3] == 'n') {
        // This one function call here fully extracts the entire AFS File.
        int ret = afs_extractFull(afs, argv[4]);
        if(ret != 0) {
            return ret;
        }
        printf("AFS extracted to %s.", argv[4]);
    }
    else {
        // If we only want to extract a single Entry,
        int entryId = atoi(argv[3]);
        // ...this function extracts the selected Entry to the folder
        char* outfile = afs_extractEntryToFile(afs, entryId, argv[4]);
        // the function returns the filepath to the extracted entry,
        // so we can just use its return 
        printf("File extracted to %s.", outfile);
        free(outfile);
    }

    // Lastly we make sure that no memory leaks occur by freeing all AFS related memory.
    afs_free(afs);
    return 0;
}
