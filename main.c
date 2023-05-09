#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "HackerEnrollment.h"

#define NUM_REQUIRED_ARGS 5

typedef struct Files {
    bool success;
    FILE* students;
    FILE* courses;
    FILE* hackers;
    FILE* queues;
    FILE* target;
} Files;

void printUsageError(const char* commandName) {
    printf("Usage: %s <flags> <students> <courses> <hackers> <queues> <target>\n", commandName);
}

// Closes the file, if it is not NULL. Calling fclose(NULL) is undefined behavior.
void tryCloseFile(FILE* file) {
    if (file != NULL) {
        fclose(file);
    }
}

void closeFiles(Files files) {
    tryCloseFile(files.students);
    tryCloseFile(files.courses);
    tryCloseFile(files.hackers);
    tryCloseFile(files.queues);
    tryCloseFile(files.target);
}

Files openFiles(const char* studentsFileName, const char* coursesFileName,
                const char* hackersFileName, const char* queuesFileName,
                const char* targetFileName)
{
    Files out = { 0 };
    out.success = true;
    out.students = fopen(studentsFileName, "r");
    out.courses = fopen(coursesFileName, "r");
    out.hackers = fopen(hackersFileName, "r");
    out.queues = fopen(queuesFileName, "r");
    out.target = fopen(targetFileName, "w");

    if (!out.students || !out.courses || !out.hackers || !out.queues || !out.target) {
        closeFiles(out);
        out.success = false;
    }

    return out;
}

int main(int argc, const char *argv[]) {
    bool caseSensitive = true;
    const char* commandName = argv[0];
    const char** primaryArgs = &argv[1];

    if (argc != NUM_REQUIRED_ARGS + 1 && argc != NUM_REQUIRED_ARGS + 2) {
        printUsageError(commandName);
        return 0;
    }

    if (argc == 6) {
        if (strcmp(primaryArgs[1], "-i") != 0) {
            printUsageError(commandName);
            return 0;
        }
        // Move over the -i.
        primaryArgs++;
        caseSensitive = false;
    }

    Files files = openFiles(
        primaryArgs[0], primaryArgs[1], primaryArgs[2], primaryArgs[3], primaryArgs[4]
    );
    
    EnrollmentSystem system = createEnrollment(files.students, files.courses, files.hackers);
    setCaseSensitive(system, caseSensitive);
    readEnrollment(system, files.queues);
    hackEnrollment(system, files.target);

    return 0;
}
