#include "HackerEnrollment.h"

#include <stdbool.h>

#define BUFFER_SIZE 256
#define COURSE_NUM_LEN 6
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

//counts elements using space amount
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

//Frees up memory associated with a student (Does not free the pointer)
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

//Frees up memory associated with a student (Does not free the pointer).
void destroyHackers(Hacker* hackers, int hackerAmount) {
    for(int i = 0; i  < hackerAmount; i++)
    {
        free(hackers[i].m_courses);
        free(hackers[i].m_friends);
        free(hackers[i].m_rivals);

        // It's good practice to NULL dangling pointers.
        hackers[i].m_courses = NULL;
        hackers[i].m_friends = NULL;
        hackers[i].m_rivals = NULL;
    }
}

//returns a course pointer based on the course number
Course* getCourseFromNum(EnrollmentSystem sys, int courseNum)
{
    for(int i = 0; i < sys->m_coursesSize; i++)
    {
        if(sys->m_courses[i].m_number == courseNum)
        {
            return &(sys->m_courses[i]);
        }
    }

    return NULL;
}

//returns a student pointer based on the id
Student* getStudentFromID(EnrollmentSystem sys, char ID[ID_SIZE + 1])
{
    for(int i = 0; i < sys->m_studentsSize; i++)
    {
        if((sys->m_students[i].m_ID, ID) == 0)
        {
            return &(sys->m_students[i]);
        }
    }

    return NULL;
}

//parses the students file and saves the information
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

//parses the courses file and saves the information
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
        course.m_queue = IsraeliQueueCreate(NULL, NULL, FRIENDSHIP_THRESHOLD, RIVALRY_THRESHOLD);

        courses[i] = course;
        i++;
    }

    *coursesSize = coursesAmount;
    return courses;
}

//parses the hackers file and saves the information
Hacker* parseHackersFile(EnrollmentSystem sys, FILE* hackersFile, int* hackersSize)
{
    bool error = false;

    char buffer[BUFFER_SIZE + 1] = { 0 };
    char tempBuffer[ID_SIZE] = { 0 };

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
        sscanf(buffer, "%s", tempBuffer);
        hacker.m_student = getStudentFromID(sys, tempBuffer);

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_courses = (Course**)malloc(sizeof(Course*) * countElementsInLine(buffer));
        error = hacker.m_courses ? error : true;
        if(hacker.m_courses)
        {
            hacker.m_coursesSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
            hacker.m_courses[j] = getCourseFromNum(sys, atoi(tempBuffer));
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_friends = (Student**)malloc(sizeof(Student*) * countElementsInLine(buffer));
        error = hacker.m_friends ? error : true;
        if(hacker.m_friends)
        {
            hacker.m_friendsSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_SIZE + 1), ID_SIZE);
            hacker.m_friends[j] = getStudentFromID(sys, tempBuffer);
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker.m_rivals = (Student**)malloc(sizeof(Student*) * countElementsInLine(buffer));
        error = hacker.m_rivals ? error : true;
        if(hacker.m_rivals)
        {
            hacker.m_rivalsSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_SIZE + 1), ID_SIZE);
            hacker.m_rivals[j] = getStudentFromID(sys, tempBuffer);
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
    sys->m_hackers = parseHackersFile(sys, hackers, &size);
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
    char buffer[BUFFER_SIZE + 1] = { 0 };
    char tempBuffer[ID_SIZE] = { 0 };
    int courseNum = 0;
    int i = 0;
    Course course = { 0 };

    while(fgets(buffer, BUFFER_SIZE, queues))
    {
        sscanf(buffer, "%d", &courseNum);
        course = *getCourseFromNum(sys, courseNum);
        for(int j = 1; j < countElementsInLine(buffer); j++)
        {
            memcpy(tempBuffer, buffer + j * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
            course.m_queue = IsraeliQueueEnqueue(course.m_queue, getStudentFromID(sys, tempBuffer));
        }
    }

    return sys;
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
