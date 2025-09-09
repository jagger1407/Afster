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
    puts("display_afl - Displays the contents of an AFL file.\n");
    puts("arg1 = A path to an AFL File");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we display the contents of an AFL file.
 *
 * This program takes in 1 argument:
 * arg1 = A path to an AFL file
*/
int main(int argc, char** argv) {
    // Checking if the argument is present
    if(argc < 2) {
        puts("ERROR: main - No AFL file specified.");
        printHelp();
        return 1;
    }

    // Checking whether the given path points to an AFL File
    int len = strlen(argv[1]);
    char* aflpath = (char*)malloc(len);
    strcpy(aflpath, argv[1]);
    if(strcmp(strlwr(aflpath) + len - 4, ".afl") != 0) {
        puts("ERROR: main - arg1 is not an AFL File.");
        printHelp();
        return 1;
    }
    free(aflpath);

    // Now that we know all arguments are there,
    // we can create an AFL Handle with the first argument
    // using the afl_open() function.
    Afl* afl = afl_open(argv[1]);
    if(afl == NULL) {
        puts("ERROR: main - afl handle couldn't be created.");
        return 1;
    }

    // Now, we create a table of all the elements within the AFL.
    // Each entry inside the AFL has a buffer of 32 aka 0x20 Bytes.
    puts(
        "+-------+----------------------------------+\n" \
        "| ID    | Name                             |\n" \
        "+-------+----------------------------------+" \
    );

    // We loop through each entry in the AFL
    for(int i=0; i < afl->head.entrycount; i++) {
        // First, we create the template line without any information in it
        char line[81];
        strcpy(line, "|       |                                  |");

        // First, the ID. Luckily we are in a for loop so we have i
        sprintf(line+2, "%5d", i);
        // remember that strncpy and sprintf both add the null terminator,
        // so we need to replace that one with a space to not end the string prematurely.
        line[7] = ' ';

        // Now, the name.
        char name[AFL_NAMEBUFFERSIZE];
        // In case the name is empty (which unlike AFS Metadata is rare for AFL files),
        // We have to check if the entry is even filled.
        if(*afl_getName(afl, i) == '\0') {
            sprintf(name, "blank_%d", i);
        }
        else {
            strncpy(name, afl_getName(afl, i), AFL_NAMEBUFFERSIZE);
        }
        strcpy(line+10, name);
        // again, replacing the null terminator so we can continue.
        line[10+strlen(name)] = ' ';

        // Now that all the information has been placed into the line,
        // we can finally print it out and move to the next entry in the AFL.
        printf("%s\n", line);
    }
    puts("+-------+----------------------------------+");

    // Lastly we make sure that no memory leaks occur by freeing all AFL related memory.
    afl_free(afl);
    return 0;
}

