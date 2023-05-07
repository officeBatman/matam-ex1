#include "HackerEnrollment.h"

#include <stdbool.h>

#define BUFFER_SIZE 256
#define COURSE_NUM_LEN 6
#define ID_LEN 9
#define SPACE_CHAR ' '


//helper functions
int getLineNum(FILE* file)
{
    int lineNum = 0;
    char buffer[BUFFER_SIZE] = { 0 };

    while(fgets(buffer, BUFFER_SIZE, file))
    {
        lineNum++;
    }

    return lineNum;
}

int countElementsInLine(char* line)
{
    int elementAmount = 0;
    while(*line)
    {
        if(SPACE_CHAR == *line)
        {
            elementAmount++;
        }
        line++;
    }

    return elementAmount == -1 ? 0 : elementAmount - 1;
}

/* Frees up memory associated with a student (Does not free the pointer).
 */
void destroyStudent(Student* student) {
    if (student == NULL) {
        return;
    }
    
    free(student->m_name);
    free(student->m_surname);
    free(student->m_city);
    free(student->m_department);

    // It's good practice to NULL dangling pointers.
    student->m_name = NULL;
    student->m_surname = NULL;
    student->m_city = NULL;
    student->m_department = NULL;
}

/* Frees up memory associated with a student (Does not free the pointer).
 */
void destroyHackers(Hacker** hackers, int hackerAmount) {
    for(int i = 0; i  < hackerAmount; i++)
    {
        if (hackers[i] == NULL)
        {
            continue;
        }
        
        free(hackers[i]->m_courseNums);
        free(hackers[i]->m_friends);
        free(hackers[i]->m_rivals);

        // It's good practice to NULL dangling pointers.
        hackers[i]->m_courseNums = NULL;
        hackers[i]->m_friends = NULL;
        hackers[i]->m_rivals = NULL;
    }
}

Student* parseStudentsFile(FILE* studentsFile, int* studentsSize)
{
    char buffer[BUFFER_SIZE + 1] = { 0 };
    int i = 0;
    int studentsAmount = getLineNum(studentsFile);
    Student* students = (Student*)malloc(sizeof(Student) * studentsAmount);
    Student student;

    student.m_name = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student.m_surname = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student.m_city = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student.m_department = (char*)malloc(sizeof(char) * BUFFER_SIZE);

    if(!(students && student.m_name && student.m_surname && student.m_city && student.m_department))
    {
        free(students);
        free(student.m_name);
        free(student.m_surname);
        free(student.m_city);
        free(student.m_department);
    }

    while(fgets(buffer, BUFFER_SIZE, studentsFile))
    {
        sscanf(buffer, "%s %d %d %s %s %s %s", student.m_ID, &student.m_credits, &student.m_GPA, student.m_name,
                                               student.m_surname, student.m_city, student.m_department);

        students[i] = student;
        i++;
    }

    *studentsSize = studentsAmount;
    return students;
}

Course* parseCoursesFile(FILE* coursesFile, int* coursesSize)
{
    char buffer[BUFFER_SIZE + 1] = { 0 };
    int i = 0;
    int coursesAmount = getLineNum(coursesFile);
    Course* courses = (Course*)malloc(sizeof(Course) * coursesAmount);
    Course course;
    if(!courses)
    {
        return NULL;
    }

    while(fgets(buffer, BUFFER_SIZE, coursesFile))
    {
        sscanf(buffer, "%d %d", &course.m_number, &course.m_size);

        courses[i] = course;
        i++;
    }

    *coursesSize = coursesAmount;
    return courses;
}

Hacker* parseHackersFile(FILE* hackersFile, int* hackersSize)
{
    bool error = false;

    char buffer[BUFFER_SIZE + 1] = { 0 };
    char tempBuffer[ID_LEN] = { 0 };

    int hackersAmount = getLineNum(hackersFile) / 4;
    Hacker* hackers = (Hacker*)malloc(sizeof(Hacker) * hackersAmount);
    Hacker hacker;
    if(!hackers)
    {
        return NULL;
    }

    for(int i = 0; i < hackersAmount; i++)
    {
        fgets(buffer, BUFFER_SIZE, hackersFile);  
        sscanf(buffer, "%s", &hacker.m_ID);

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_courseNums = (unsigned int*)malloc(sizeof(unsigned int) * countElementsInLine(buffer));
        error = hacker.m_courseNums ? error : true;
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
            sscanf(tempBuffer, "%d", &hacker.m_courseNums[j]);
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_friends = (char*)malloc(sizeof(char) * countElementsInLine(buffer));
        error = hacker.m_friends ? error : true;
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_LEN + 1), ID_LEN);
            sscanf(tempBuffer, "%s", &hacker.m_friends[j]);
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_rivals = (char*)malloc(sizeof(char) * countElementsInLine(buffer));
        error = hacker.m_rivals ? error : true;
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_LEN + 1), ID_LEN);
            sscanf(tempBuffer, "%s", &hacker.m_rivals[j]);
        }

        hackers[i] = hacker;
    }

    if(error)
    {
        destroyHackers(hackers, hackersAmount);
        return NULL;
    }

    *hackersSize = hackersAmount;
    return hackers;
}

/* Clones a string to a new buffer (that should be freed later). */
char* cloneString(const char* string) {
    int size = 0;
    char* ret = NULL;

    ret = (char*)malloc(sizeof(char) * (strlen(string) + 1));
    if (ret == NULL) {
        return NULL;
    }

    strcpy(ret, string);

    return ret;
}

/* Clones a student (all memory associated with it). */
Student cloneStudent(const Student* student, bool* success) {
    Student out = { 0 };
    out.m_credits = student->m_credits;
    out.m_GPA = student->m_GPA;
    out.m_ID = cloneString(student->m_ID);
    out.m_name = cloneString(student->m_name);
    out.m_surname = cloneString(student->m_surname);
    out.m_city = cloneString(student->m_city);
    out.m_department = cloneString(student->m_department);
    if (!out.m_name || !out.m_surname || !out.m_city || !out.m_department || !out.m_ID) {
        *success = false;
        free(out.m_ID);
        free(out.m_name);
        free(out.m_surname);
        free(out.m_city);
        free(out.m_department);
    }
    *success = true;
    return out;
}

//header implementations
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers)
{
    EnrollmentSystem sys = (EnrollmentSystem)malloc(sizeof(struct EnrollmentSystem_t));
    int size = 0;

    if (!sys)
    {
        return NULL;
    }

    sys->m_students = parseStudentsFile(students, &size);
    sys->m_studentsSize = size;
    sys->m_courses = parseCoursesFile(courses, &size);
    sys->m_coursesSize = size;
    sys->m_hackers = parseHackersFile(hackers, &size);
    sys->m_hackersSize = size;

    if(!sys->m_students || !sys->m_courses || !sys->m_hackers)
    {
        free(sys->m_students);
        free(sys->m_courses);
        free(sys->m_hackers);
        free(sys);
        return NULL;
    }

    return sys;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues)
{
    EnrollmentSystem out = (EnrollmentSystem)malloc(sizeof(struct EnrollmentSystem_t));
    int courseNumber = 0;
    char studentID[BUFFER_SIZE] = { 0 };
    int i = 0;
    bool success = true;
    bool studentFound = false;

    out->m_studentsSize = 0;
    out->m_coursesSize = 0;
    out->m_hackersSize = 0;
    // Initialize dynamic memory.
    out->m_courses = (Course*)malloc(0);
    out->m_hackers = (Hacker*)malloc(0);
    // Start students from maximum size, and realloc at the end for memory.
    out->m_students = (Student*)malloc(sizeof(Student*) * sys->m_studentsSize);

    // <Course Number> (<Student ID>)*
    fscanf(queues, "%d", &courseNumber);

    // Read students in a loop.
    while (success && fscanf(queues, "%s", studentID) == 1) {
        studentFound = false;
        for (i = 0; i < sys->m_studentsSize && !studentFound && success; i++) {
            if (strcmp(sys->m_students[i].m_ID, studentID) == 0) {
                out->m_students[out->m_studentsSize] = cloneStudent(&sys->m_students[i], &success);
                if (success) {
                    out->m_studentsSize++;
                    studentFound = true;
                }
            }
        }
    }

    if (!success) {
        destroyEnrollment(out);
        out = NULL;
    }

    return out;
}

void hackEnrollment(EnrollmentSystem sys, FILE* out)
{

}

void destroyEnrollment(EnrollmentSystem enrollment) {
    int i = 0;
    
    // Do nothing for null.
    if (enrollment == NULL) {
        return;
    }

    // Free each student indivudially because they hold memory.
    for (i = 0; i < enrollment->m_studentsSize; i++) {
        destroyStudent(&enrollment->m_students[i]);
    }
    free(enrollment->m_students);

    // Course instances and hacker instances do not hold memory. 
    // TODO: Now hackers hold memory! Free it.
    free(enrollment->m_courses);
    free(enrollment->m_hackers);

    // It's good practice to NULL dangling pointers.
    enrollment->m_students = NULL;
    enrollment->m_courses = NULL;
    enrollment->m_hackers = NULL;

    free(enrollment);
}
