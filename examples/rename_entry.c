#include <stdio.h>
#include <stdlib.h>
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
    puts("rename - Rename an entry of the AFS.\n");
    puts("arg1 = A path to an AFS File");
    puts("arg2 = The index of the entry whose name we're changing");
    puts("arg3 = The new name.");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we rename one singular entry within the AFS.
 *
 * This program takes in 3 arguments:
 * arg1 = A path to an AFS file
 * arg2 = The index of the entry whose name we're changing
 * arg3 = The new name.
*/
int main(int argc, char** argv) {
    // Checking if all arguments are present
    if(argc < 2) {
        puts("ERROR: main - No AFS file specified.");
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

    // We convert the given index into an int
    int entryId = atoi(argv[2]);

    // Here we rename the entry. If everything turned out correctly,
    // The 'true' signals that this change will be permanently written to the file.
    // We store the return code to handle potential errors.
    int returnCode = afs_renameEntry(afs, entryId, argv[3], true);

    // If anything went wrong while this function was changing the name,
    // The return code will let us know what exactly happened.
    if(returnCode != 0) {
        // Free whatever memory we've already allocated
        afs_free(afs);
        return 4;
    }

    // Lastly we make sure that no memory leaks occur by freeing all AFS related memory.
    afs_free(afs);
    return 0;
}
