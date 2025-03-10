#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    puts("display - Display all the entires within the AFS.\n");
    puts("arg1 = A path to an AFS file");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we take as an argument a path to an AFS file, and display
 * all of its contents inside of a table.
*/
int main(int argc, char** argv) {
    // We first check whether the AFS argument even exists.
    if(argc < 2) {
        puts("ERROR: main - No AFS file specified.");
        printHelp();
        return 1;
    }
    // We then check whether this argument links to an AFS file
    // We take the length of the given filepath
    int len = strlen(argv[1]);
    char afspath[len];
    strcpy(afspath, argv[1]);
    // strlwr turns the string into all lowercase,
    // then we strcmp the last 4 letters to be ".afs"
    if(strcmp(strlwr(afspath) + len - 4, ".afs") != 0) {
        puts("ERROR: main - arg1 is not an AFS File.");
        printHelp();
        return 1;
    }

    // If it does, we create the handle using afs_open().
    Afs* afs = afs_open(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 2;
    }

    // Now, we create a table of all the elements within the AFS.
    // The metadata section shows that the maximum size of a name is 0x20
    // aka 32 Bytes, hence the 32 char length of the 'Name' column here.
    puts(
        "+-------+----------------------------------+------------+---------------------+\n" \
        "| ID    | Name                             | Size       | Last Modified Date  |\n" \
        "+-------+----------------------------------+------------+---------------------+" \
    );

    // We loop through each entry in the AFS
    for(int i=0;i<afs->header.entrycount;i++) {
        // First, we create the template line without any information in it
        char line[81];
        strcpy(line, "|       |                                  |            |                     |");

        // First, the ID. Luckily we are in a for loop so we have i
        sprintf(line+2, "%5d", i);
        // remember that strncpy and sprintf both add the null terminator,
        // so we need to replace that one with a space to not end the string prematurely.
        line[7] = ' ';

        // Now, the name.
        char name[AFSMETA_NAMEBUFFERSIZE];
        // In many cases, the name portion of the metadata is garbled, or
        // Completely empty. For the latter, we need to create a string ourselves.
        if(*afs->meta[i].filename == '\0') {
            sprintf(name, "blank_%d", i);
        }
        else {
            strncpy(name, afs->meta[i].filename, 32);
        }
        strcpy(line+10, name);
        // again, replacing the null terminator so we can continue.
        line[10+strlen(name)] = ' ';

        // Then we add the size
        sprintf(line+45, "%10d", afs->header.entryinfo[i].size);
        line[55] = ' ';

        // And here we finally add the last modified date.
        // This one is using a proprietary format, so we'll be using
        // The getter and conversion functions within the library.
        Timestamp t = afs_getLastModifiedDate(afs, i);
        char* lastMod = afs_timestampToString(t);
        strncpy(line+58, lastMod, 20);
        line[strlen(lastMod)+58] = ' ';
        // I feel it's important to mention that because afs_timestampToString()
        // returns a char* that was allocated in said function, it is necessary to
        // free it after usage in order to ensure memory safety.
        free(lastMod);

        // Now that all the information has been placed into the line,
        // we can finally print it out and move to the next entry in the AFS.
        printf("%s\n", line);
    }
    puts("+-------+----------------------------------+------------+---------------------+");

    return 0;
}
