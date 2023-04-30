#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct Files {
    FILE* students;
    FILE* courses;
    FILE* hackers;
    FILE* target;
    FILE* out;
} Files;

void printError(const char* commandName) {
    printf("Usage: %s <flags> <students> <courses> <hackers> <target>\n", commandName);
}

int main(int argc, const char *argv[]) {
    bool caseSensitive = true;

    const char** primaryArgs = argv[1];

    if (argc != 5 || argc != 6) {
        printError(argv[0]);
        return 0;
    }

    if (argc == 6) {
        // Move over the -i.
        argv++;
        if (strcmp(argv[1], "-i") == 0) {
            caseSensitive = false;
        } else {
            printError(argv[0]);
            return 0;
        }
    }

    Files files = openFiles(argv

    return 0;
}
