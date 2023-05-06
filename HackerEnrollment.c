#include "HackerEnrollment.h"

//helper functions
Student* parseStudentsFile(FILE* studentsFile, int* studentsSize)
{
    char* buffer[BUFFER_SIZE] = { 0 };
    int studentsAmount = getLineNum(studentsFile);
    Student* students = (Student*)malloc(sizeof(Student) * studentsAmount);
    if(!students)
    {
        return NULL;
    }

    int studentsAmount = 0;

    while(fgets(buffer, BUFFER_SIZE, students))
    {
        studentsAmount++;
    }

    *studentsSize = studentsAmount;
}

Course* parseCoursesFile(FILE* coursesFile, int* coursesSize)
{

}

Hacker* parseHackersFile(FILE* hackersFile, int* hackersSize)
{

}

//header implementations
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers)
{
    EnrollmentSystem sys = { 0 };
    int size = 0;

    sys.m_students = parseStudentsFile(students, &size);
    sys.m_studentsSize = size;
    sys.m_courses = parseCoursesFile(courses, &size);
    sys.m_coursesSize = size;
    sys.m_hackers = parseHackersFile(hackers, &size);
    sys.m_hackersSize = size;

    if(!sys.m_students || !sys.m_courses || !sys.m_hackers)
    {
        EnrollmentSystem error = { 0 };
        return error;
    }

    return sys;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues)
{

}

void hackEnrollment(EnrollmentSystem sys, FILE* out)
{

}