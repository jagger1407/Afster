#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    puts("replace - Replace one singular entry from an AFS File.\n");
    puts("arg1 = A path to an AFS file");
    puts("arg2 = The index of the entry that we're replacing");
    puts("arg3 = A path to the file we're putting in");
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we replace one singular entry within the AFS.
 *
 * This program takes in 3 arguments:
 * arg1 = A path to an AFS file
 * arg2 = The index of the entry that we're replacing
 * arg3 = A path to the file we're putting in
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
        return 2;
    }
    else if(argc < 4) {
        puts("ERROR: main - No replacement file specified.");
        return 3;
    }
    // Checking if arg1 contains a path to an AFS File
    // We do that by copying the string so the filepath remains unaffected
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
    // Checking if arg3 contains a valid string
    if(argv[3] == NULL || *argv[3] == 0x00) {
        puts("ERROR: main - Replacement Filepath invalid.");
    }

    // Now that we know all arguments are there and valid enough,
    // we can create an AFS Handle with the first argument
    // using the afs_open() function.
    Afs* afs = afs_open(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 1;
    }

    // We convert the given index into an int
    int entryId = atoi(argv[2]);

    // We open the file we want to replace
    FILE* file = fopen(argv[3], "rb");

    // We get the filesize
    int start = ftell(file);
    fseek(file, 0, SEEK_END);
    int end = ftell(file);
    int filesize = end - start;
    printf("Original File size is %d Bytes.\n", filesize);

    puts("Replacing file...");

    // Now, we create a buffer with that filesize.
    // Since malloc is used, this buffer must be freed at the end.
    u8* buffer = (u8*)malloc(filesize);
    // and now the contents of the file will be read into the buffer
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, filesize, file);

    // Now, let's grab the filename so we can update the AFS accordingly.
    // This is for Unix-Style filepaths.
    char* unix_end = strrchr(argv[3], '/');
    // This is for Windows-Style filepaths.
    char* win_end = strrchr(argv[3], '\\');
    char* filename;
    // This is to decide which one to go for
    if(win_end > unix_end) filename = win_end;
    else filename = unix_end; 
    // If there is no / char in the string, only the filename has been given since it's in the same path.
    if(filename == NULL) filename = argv[3];
    // strrchr() returns a pointer to the found char, since we only want what comes after the slash, we increment the pointer.
    else filename++;

    // Here, the magic happens.
    // We use afs_replaceEntry() to put our file into the AFS.
    int ret = afs_replaceEntry(afs, entryId, buffer, filesize);

    // The function will return >0 if an error happened, or 0 if all went well
    if(ret != 0) {
        puts("Error happened in afs_replaceEntry.");
        printf("ret = %d\n", ret);
        return 4;
    }
    else {
        puts("File successfully replaced.");
        // Just for fun, let's update the metadata to our current time too.
        puts("Changing metadata...");
    }

    // To do this, we first get this entries Metadata using afs_getEntryMetadata().
    AfsEntryMetadata meta = afs_getEntryMetadata(afs, entryId);

    // We then copy over the name
    strncpy(meta.filename, filename, AFSMETA_NAMEBUFFERSIZE);

    // Now let's update the time.
    // First, we get our current time as a struct tm
    time_t current_time = time(NULL);
    struct tm tm = *localtime(&current_time);
    // Then, we set each member to the current time
    meta.lastModified.year = tm.tm_year + 1900;
    meta.lastModified.month = tm.tm_mon + 1;
    meta.lastModified.day = tm.tm_mday;
    meta.lastModified.hours = tm.tm_hour;
    meta.lastModified.minutes = tm.tm_min;
    meta.lastModified.seconds = tm.tm_sec;

    // With this info done, we can update the entry in the AFS using the corresponding setter.
    afs_setEntryMetadata(afs, entryId, meta, true);

    puts("Metadata changed.");

    // Lastly we make sure that no memory leaks occur by freeing all AFS related memory.
    afs_free(afs);
    // ...and the buffer for the file data that we created earlier.
    free(buffer);
    return 0;
}

