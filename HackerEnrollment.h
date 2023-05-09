#ifndef HACKER_ENROLLMENT_H_
#define HACKER_ENROLLMENT_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "IsraeliQueue.h"


#define ID_SIZE 9
#define FRIENDSHIP_THRESHOLD 20
#define RIVALRY_THRESHOLD 0

typedef struct Student_t * Student;
typedef struct Course_t * Course;
typedef struct Hacker_t * Hacker;
typedef struct EnrollmentSystem_t * EnrollmentSystem;

typedef struct Student_t {
    char m_ID[ID_SIZE + 1];
    unsigned int m_credits;
    unsigned int m_GPA;
    char* m_name;
    char* m_surname;
    char* m_city;
    char* m_department;
    Hacker m_hacker;
} Student_t;

typedef struct Course_t {
    unsigned int m_number;
    unsigned int m_size;
    IsraeliQueue m_queue;
} Course_t;

typedef struct Hacker_t {
    Student m_student;
    Course* m_courses;
    int m_coursesSize;
    Student* m_friends;
    int m_friendsSize;
    Student* m_rivals;
    int m_rivalsSize;
} Hacker_t;

typedef struct EnrollmentSystem_t {
    Student* m_students;
    int m_studentsSize;
    Course* m_courses;
    int m_coursesSize;
    Hacker* m_hackers;
    int m_hackersSize;
} EnrollmentSystem_t;


EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

void hackEnrollment(EnrollmentSystem sys, FILE* out);

/* Frees up all the memory associated with the given EnrollmentSystem instance.
 */
void destroyEnrollment(EnrollmentSystem enrollment);


#endif
