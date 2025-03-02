#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../afs.h"

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
    // Checking if arg3 contains a valid string
    if(argv[3] == NULL || *argv[3] == 0x00) {
        puts("ERROR: main - Replacement Filepath invalid.");
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

    // We open the file we want to replace
    FILE* file = fopen(argv[3], "rb");

    // We get the filesize
    int start = ftell(file);
    fseek(file, 0, SEEK_END);
    int end = ftell(file);
    int filesize = end - start;
    printf("Original File size is %d Bytes.\n", filesize);
    puts("Replacing file...");

    // Now, we create a buffer with that filesize
    u8 buffer[filesize];
    // ...and read the contents of the file into that buffer
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, filesize, file);

    // Now, let's grab the filename so we can update the AFS accordingly.
    // Note: This is for Unix-Style filepaths.
    char* filename = strrchr(argv[3], '/');
    // If there is no / char in the string, only the filename has been given since it's in the same path.
    if(filename == NULL) filename = argv[3];
    // strrchr() returns a pointer to the found char, since we only want what comes after the slash, we increment the pointer.
    else filename++;

    // Here, the magic happens.
    // We use afs_replaceEntry() to put our file into the AFS.
    int ret = afs_replaceEntry(afs, entryId, buffer, filesize);
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
    return 0;
}

