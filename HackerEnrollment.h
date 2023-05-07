#ifndef HACKER_ENROLLMENT_H_
#define HACKER_ENROLLMENT_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    char* m_ID;
    unsigned int m_credits;
    unsigned int m_GPA;
    char* m_name;
    char* m_surname;
    char* m_city;
    char* m_department;
} Student;

typedef struct {
    unsigned int m_number;
    unsigned int m_size;
} Course;

typedef struct {
    char* m_ID;
    unsigned int* m_courseNums;
    char* m_friends;
    char* m_rivals;
} Hacker;

typedef struct EnrollmentSystem_t {
    Student* m_students;
    int m_studentsSize;
    Course* m_courses;
    int m_coursesSize;
    Hacker* m_hackers;
    int m_hackersSize;
} EnrollmentSystem_t;

typedef struct EnrollmentSystem_t * EnrollmentSystem;



EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

void hackEnrollment(EnrollmentSystem sys, FILE* out);

/* Frees up all the memory associated with the given EnrollmentSystem instance.
 */
void destroyEnrollment(EnrollmentSystem enrollment);


#endif