#include <stdio.h>
#include <stdlib.h>
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
    puts("rename_afl - Rename an entry of the AFL.\n");
    puts("arg1 = A path to an AFL File");
    puts("arg2 = The index of the entry whose name we're changing");
    puts("arg3 = The new name.");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we rename one singular entry within the AFL.
 *
 * This program takes in 3 arguments:
 * arg1 = A path to an AFL file
 * arg2 = The index of the entry whose name we're changing
 * arg3 = The new name.
*/
int main(int argc, char** argv) {
    // Checking if all arguments are present
    if(argc < 2) {
        puts("ERROR: main - No AFL file specified.");
        printHelp();
        return 1;
    }
    else if(argc < 3) {
        puts("ERROR: main - No entry index specified.");
        printHelp();
        return 2;
    }
    else if(argc < 4) {
        puts("ERROR: main - No name specified.");
        printHelp();
        return 3;
    }
    // Checking whether the given path points to an AFL File
    int len = strlen(argv[1]);
    char afspath[len];
    strcpy(afspath, argv[1]);
    if(strcmp(strlwr(afspath) + len - 4, ".afl") != 0) {
        puts("ERROR: main - arg1 is not an AFL File.");
        printHelp();
        return 1;
    }

    // Now that we know all arguments are there,
    // we can create an AFL Handle with the first argument
    // using the afl_open() function.
    Afl* afl = afl_open(argv[1]);
    if(afl == NULL) {
        puts("ERROR: main - afl handle couldn't be created.");
        return 1;
    }

    // We convert the given index into an int
    int entryId = atoi(argv[2]);

    // Here we rename the entry.
    int returnCode = afl_rename(afl, entryId, argv[3]);

    // If anything went wrong while this function was changing the name,
    // The return code will let us know what exactly happened.
    if(returnCode != 0) {
        // Free whatever memory we've already allocated
        afl_free(afl);
        return 4;
    }

    // Now, the entry has been renamed. Great!
    // But only inside the in-memory Afl struct.
    // To permanently change it, we'll save the file using afl_save().
    afl_save(afl);

    // Lastly we make sure that no memory leaks occur by freeing all AFL related memory.
    afl_free(afl);
    return 0;
}
