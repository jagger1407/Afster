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

/** Prints a help text explaining how to use this program.
 */
void printHelp() {
    puts("import_folder - Replaces name-matching AFS entries with files from a given folder.\n");
    puts("arg1 = A path to an AFS file");
    puts("arg2 = A path to the folder that should be imported.");
}

#ifdef _WIN32

#include <windows.h>

/** Retrieves all files in a given directory.
 *
 * @param directory_path The path to the directory
 * @param file_count pointer to an integer where the file count will be written to.
 * @return char*[file_count] containing each filename.
 * @note return value must be freed with freeDirFileList().
 */
char** getFilesInDirectory(const char* directory_path, int* file_count) {
    WIN32_FIND_DATA found_file;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char* dir = NULL;

    // First, we have to prepare the string for usage
    // if the string ends in a \, we need to add the * too
    if(directory_path[strlen(directory_path)-1] == '\\') {
        dir = (char*)malloc(strlen(directory_path) + 2);
        memset(dir, 0x00, strlen(directory_path) + 2);
        strcpy(dir, directory_path);
        strcat(dir, "*");
    }
    // if the string ends in a *, the string should already be what it's meant to be
    else if(directory_path[strlen(directory_path)-1] == '*') {
        dir = directory_path;
    }
    // if it ends in neither, we need to add "\*"
    else {
        dir = (char*)malloc(strlen(directory_path) + 3);
        memset(dir, 0x00, strlen(directory_path) + 3);
        strcpy(dir, directory_path);
        strcat(dir, "\\*");
    }

    // We needed the string to end in "\*" because of the way FindFirstFile() works.
    // If it doesn't have a \, it thinks the directory is a file.
    // If it doesn't have a *, it doesn't select all the subfiles in the directory.
    hFind = FindFirstFile(dir, &found_file);
    if (INVALID_HANDLE_VALUE == hFind) {
          puts("ERROR: - getFilesInDirectory - can't read directory.");
          printf("Used dir value: %s\n", dir);
          free(dir);
          return NULL;
    }

    // Initialize file_count to 0
    *file_count = 0;

    // First, we need to count how many files we have.
    // To do this, we loop through each entry in the directory using FindNextFile()
    do {
        // subdirectories should be skipped.
        if(found_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        // Increment the file counter
        (*file_count)++;
    } while(FindNextFile(hFind, &found_file) != 0);
    // I'm not sure this is necessary,
    // but the example I based my code on used it so I will too.
    FindClose(hFind);

    // Now, we create the actual file list using the file count we created above.
    char** file_list = (char**)malloc(sizeof(char*) * *file_count);

    hFind = FindFirstFile(dir, &found_file);
    if (INVALID_HANDLE_VALUE == hFind) {
          puts("ERROR: - getFilesInDirectory - can't read directory.");
          printf("Used dir value: %s\n", dir);
          free(dir);
          return NULL;
    }
    // Next, for each file in the directory, we get its file name and add it to the file list.
    int i=0;
    do {
        if(found_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        int nameLen = strlen(found_file.cFileName) + 1;
        char* name = (char*)malloc(nameLen);
        memset(name, 0x00, nameLen);

        strncpy(name, found_file.cFileName, nameLen);
        file_list[i] = name;
        i++;
    } while(FindNextFile(hFind, &found_file) != 0);
    FindClose(hFind);


    free(dir);
    return file_list;
}

#elif defined(__unix__)

#include <dirent.h>

#define _BSD_SOURCE

/** Retrieves all files in a given directory.
 *
 * @param directory_path The path to the directory
 * @param file_count pointer to an integer where the file count will be written to.
 * @return char*[file_count] containing each filename.
 * @note return value must be freed with freeDirFileList().
 */
char** getFilesInDirectory(const char* directory_path, int* file_count) {
    // First, we open the directory
    DIR* dir = opendir(directory_path);
    if(dir == NULL) {
        puts("ERROR: getFilesInDirectory - opendir failed.");
        perror("opendir");
        return NULL;
    }
    // We initialize the file count to 0.
    *file_count = 0;

    // Let's first count how many files we even have.

    // This variable will house each file while we loop through the directory
    struct dirent* curEntry;
    // readdir() writes the next file into curEntry
    while((curEntry = readdir(dir)) != NULL) {
        // if the entry is "." (current), ".." (directory up),
        // or it has the type "directory", it's not a file and should thus be discarded.
        if (strcmp(curEntry->d_name, ".") == 0 ||
            strcmp(curEntry->d_name, "..") == 0 ||
            curEntry->d_type == DT_DIR) {
            continue;
        }
        (*file_count)++;
    }
    // Now that we know that, let's rewind the dir pointer
    // back to the beginning of the directory.
    rewinddir(dir);

    // Finally, we can start reading the files.
    // To do this, we first create the file list,
    char** file_list = (char**)malloc(sizeof(char*) * *file_count);
    // ...and an index for this list.
    int i=0;
    // Now, we can start looping through each entry again.
    while((curEntry = readdir(dir)) != NULL) {
        // if the entry is "." (current), ".." (directory up),
        // or it has the type "directory", it's not a file and should thus be discarded.
        if (strcmp(curEntry->d_name, ".") == 0 ||
            strcmp(curEntry->d_name, "..") == 0 ||
            curEntry->d_type == DT_DIR) {
            continue;
        }

        // It's time to allocate memory to store the filename.
        // Note: the + 1 at the end there is to fit a null terminator.
        int namelen = strlen(curEntry->d_name) + 1;
        file_list[i] = (char*)malloc(namelen);
        // And clear whatever garbage data is in it right now.
        memset(file_list[i], 0x00, namelen);
        // All that's left to do now is to put the name in here.
        strcpy(file_list[i], curEntry->d_name);
        i++;
    }
    // To not waste memory we free the directory and return the created file list.
    closedir(dir);
    return file_list;
}

#endif

/** Frees a directory file list as created by getFilesInDirectory().
 *
 * @param file_list The file list that should be freed
 * @param file_count The amount of files inside this list
 */
void freeDirFileList(char** file_list, int file_count) {
    // As we malloc'd each string inside the list,
    // we also need to free each one of those to ensure no memory has leaked.
    for(int i=0;i<file_count;i++) {
        free(file_list[i]);
    }
    // Then we free the file list itself
    free(file_list);
    // Finally, we set it to NULL so that it doesn't even point to anything anymore.
    file_list = NULL;
}

/*
 * This is an example program used to demonstrate how one can use this library.
 * In this case, we replace all matching files within the AFS with files in a given folder.
 *
 * This program takes in 3 arguments:
 * arg1 = A path to an AFS file
 * arg2 = A path to the folder that should be imported
*/
int main(int argc, char** argv) {
    // Checking if all arguments are present
    if(argc < 2) {
        puts("ERROR: main - No AFS file specified.");
        printHelp();
        return 1;
    }
    else if(argc < 3) {
        puts("ERROR: main - No import folder specified.");
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

    if(argv[2] == NULL || *argv[2] == 0x00) {
        puts("ERROR: main - Output folder invalid.");
        return 2;
    }

    // Now that we know all arguments are there,
    // we can create an AFS Handle with the first argument
    // using the afs_open() function.
    Afs* afs = afs_open(argv[1]);
    if(afs == NULL) {
        puts("ERROR: main - AFS file couldn't be created.");
        return 1;
    }

    // First, we need to get all the files in the given directory.
    puts("Getting all files from the directory...");
    int amountFiles = 0;
    char** dirFiles = getFilesInDirectory(argv[2], &amountFiles);

    // If the given directory can't be read for some reason
    if(dirFiles == NULL) {
        puts("ERROR: main - couldn't open directory.");
    }

    puts("Files retrived:");
    for(int i=0;i<amountFiles;i++) {
        printf("%s\n", dirFiles[i]);
    }

    // Since we now know how many files to replace,
    //we need to make an int array for their IDs.
    int entryIds[amountFiles];
    for(int i=0;i<amountFiles;i++) {
        entryIds[i] = -1;
    }
    // This might seem weird, but we need to create yet another string array.
    char* filePaths[amountFiles];
    // I'll explain both of these later.

    puts("Finding corresponding AFS entries...");
    // Now that we have the files, we have to see if they're in the AFS.
    // To do this, we first loop through the file list...
    for(int dirIdx = 0; dirIdx < amountFiles; dirIdx++) {
        char* name = dirFiles[dirIdx];
        // ...and then we loop through each entry in the AFS.
        for(int afsIdx = 0; afsIdx < afs->header.entrycount; afsIdx++) {
            if(strcmp(name, afs->meta[afsIdx].filename) == 0) {
                // if the file was found in the AFS, we add its index to the entry ID array.
                entryIds[dirIdx] = afsIdx;
                // we also need to add the file path to the file path array.
                // in order to do this, we get the length of the directory path
                int dirlen = strlen(argv[2]);
                // Now, we allocate memory for this path string.
                // The 2 bytes added are for the null terminator and a potential slash.
                filePaths[dirIdx] = (char*)malloc(dirlen + strlen(name) + 2);
                memset(filePaths[dirIdx], 0x00, dirlen + strlen(name) + 2);
                // Of course, first the path must be copied into this string
                strncpy(filePaths[dirIdx], argv[2], dirlen);
                // Now, there's 2 potential ways for this path string to look
                // 1. /path/to/dir/
                // 2. /path/to/dir
                // The difference being the slash at the end.
                // Assuming this slash exists would break the program if it didn't,
                // so to be as safe as possible, we need to manually add it.
                if(strrchr(filePaths[dirIdx], '\\') > strrchr(filePaths[dirIdx], '/')) {
                    if(filePaths[dirIdx][dirlen-1] != '\\') {
                        filePaths[dirIdx][dirlen] = '\\';
                    }
                }
                else {
                    if(filePaths[dirIdx][dirlen-1] != '/') {
                        filePaths[dirIdx][dirlen] = '/';
                    }
                }
                // And now, the file name can be added.
                strcat(filePaths[dirIdx], name);
                break;
            }
        }
        if(entryIds[dirIdx] == -1) {
            printf("WARNING: File '%s' was not found in AFS.\n", name);
        }
    }

    puts("Found AFS Entries:");
    for(int i=0;i<amountFiles;i++) {
        printf("Entry ID %d\n", entryIds[i]);
    }

    // Now, we found all the entries in the AFS.
    // All that's left is to tell the program to put them into the AFS.
    puts("Rebuilding AFS with new Files...");
    // To do that, we need to use afs_replaceEntriesFromFiles().
    // So, how does this function work?
    // It takes the entry IDs of the files we want to replace,
    // the filepaths to the files for each of those we want to replace them with
    // And the total amount.
    // This function rebuilds the entire AFS so it might take a while.
    int ret = afs_replaceEntriesFromFiles(afs, entryIds, filePaths, amountFiles);
    if(ret != 0) {
        puts("ERROR: main - Replacing the files threw an error.");
        printf("Error Code %d\n", ret);
    }
    else {
        puts("AFS Successfully built!");
    }

    // We free the directory file list after creating it.
    freeDirFileList(dirFiles, amountFiles);
    // Lastly we make sure that no memory leaks occur by freeing all AFS related memory.
    afs_free(afs);
    return 0;
}

