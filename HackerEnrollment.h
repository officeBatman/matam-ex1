#ifndef HACKER_ENROLLMENT_H_
#define HACKER_ENROLLMENT_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 256


typedef struct {
    int m_ID;
    unsigned int m_credits;
    float m_GPA;
    char* m_name;
    char* m_surname;
    char* m_city;
    char* m_department;
} Student;

typedef struct {
    int m_number;
    int m_size;
} Course;

typedef struct {
    int m_ID;
    int* m_courseNums;
    Student* m_friends;
    Student* m_rivals;
} Hacker;

typedef struct {
    Student* m_students;
    int m_studentsSize;
    Course* m_courses;
    int m_coursesSize;
    Hacker* m_hackers;
    int m_hackersSize;
} EnrollmentSystem;


EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

void hackEnrollment(EnrollmentSystem sys, FILE* out);


#endif