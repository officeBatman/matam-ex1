#include "HackerEnrollment.h"

#include <stdbool.h>

#define BUFFER_SIZE 256
#define COURSE_NUM_LEN 6
#define SPACE_CHAR ' '

#define MAX(X, Y) (((X) < (Y)) ? (X) : (Y))

//helper functions
int getLineNum(FILE* file)
{
    int lineNum = 0;
    char buffer[BUFFER_SIZE] = { 0 };

    while(fgets(buffer, BUFFER_SIZE, file))
    {
        lineNum++;
    }

    // Push the file pointer back to the start of the file.
    fseek(file, 0, SEEK_SET);

    return lineNum;
}

//counts elements using space amount
int countElementsInLine(char* line)
{
    int elementAmount = 1;
    while(*line)
    {
        if(SPACE_CHAR == *line)
        {
            elementAmount++;
        }
        line++;
    }

    return elementAmount;
}

//Frees up memory associated with a student.
void destroyStudent(Student student) {
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

    free(student);
}

void destroyHacker(Hacker hacker) {
    if (hacker == NULL) {
        return;
    }

    // Don't free the instances in the array - the EnrollmentSystem is responsible for that.
    free(hacker->m_courses);
    free(hacker->m_friends);
    free(hacker->m_rivals);

    hacker->m_courses = NULL;
    hacker->m_friends = NULL;
    hacker->m_rivals = NULL;

    free(hacker);
}

void destroyHackers(Hacker* hackers, int hackerAmount) {
    for(int i = 0; i  < hackerAmount; i++)
    {
        destroyHacker(hackers[i]);
    }
}

void destroyCourse(Course course) {
    if (course == NULL) {
        return;
    }

    IsraeliQueueDestroy(course->m_queue);

    free(course);
}

//returns a course pointer based on the course number
Course getCourseFromNum(EnrollmentSystem sys, int courseNum)
{
    for(int i = 0; i < sys->m_coursesSize; i++)
    {
        if(sys->m_courses[i]->m_number == courseNum)
        {
            return sys->m_courses[i];
        }
    }

    return NULL;
}

//returns a student pointer based on the id
Student getStudentFromID(EnrollmentSystem sys, char ID[ID_SIZE + 1])
{
    for(int i = 0; i < sys->m_studentsSize; i++)
    {
        if(strcmp(sys->m_students[i]->m_ID, ID) == 0)
        {
            return sys->m_students[i];
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

    student->m_name = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student->m_surname = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student->m_city = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    student->m_department = (char*)malloc(sizeof(char) * BUFFER_SIZE);

    if(!(students && student->m_name && student->m_surname && student->m_city && student->m_department))
    {
        free(students);
        free(student->m_name);
        free(student->m_surname);
        free(student->m_city);
        free(student->m_department);
    }

    while(fgets(buffer, BUFFER_SIZE, studentsFile))
    {
        sscanf(buffer, "%s %d %d %s %s %s %s",
        student->m_ID, &student->m_credits, &student->m_GPA, student->m_name,
        student->m_surname, student->m_city, student->m_department);
        student->m_hacker = NULL;

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
        sscanf(buffer, "%d %d", &course->m_number, &course->m_size);
        course->m_queue = IsraeliQueueCreate(NULL, NULL, FRIENDSHIP_THRESHOLD, RIVALRY_THRESHOLD);

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
        hacker->m_student = getStudentFromID(sys, tempBuffer);
        hacker->m_student->m_hacker = &hacker;

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker->m_courses = (Course*)malloc(sizeof(Course) * countElementsInLine(buffer));
        error = hacker->m_courses ? error : true;
        if(hacker->m_courses)
        {
            hacker->m_coursesSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
            hacker->m_courses[j] = getCourseFromNum(sys, atoi(tempBuffer));
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker->m_friends = (Student*)malloc(sizeof(Student) * countElementsInLine(buffer));
        error = hacker->m_friends ? error : true;
        if(hacker->m_friends)
        {
            hacker->m_friendsSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_SIZE + 1), ID_SIZE);
            hacker->m_friends[j] = getStudentFromID(sys, tempBuffer);
        }

        fgets(buffer, BUFFER_SIZE, hackersFile);
        hacker->m_rivals = (Student*)malloc(sizeof(Student) * countElementsInLine(buffer));
        error = hacker->m_rivals ? error : true;
        if(hacker->m_rivals)
        {
            hacker->m_rivalsSize = countElementsInLine(buffer);
        }
        for(int j = 0; j < countElementsInLine(buffer) && !error; j++)
        {
            memcpy(tempBuffer, buffer + j * (ID_SIZE + 1), ID_SIZE);
            hacker->m_rivals[j] = getStudentFromID(sys, tempBuffer);
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

//check if a hacker is friends with a given student
bool checkFriend(Hacker hacker, Student student)
{
    for(int i = 0; i < hacker->m_friendsSize; i++)
    {
        if(hacker->m_friends[i] == student)
        {
            return true;
        }
    }

    return false;
}

//check if a hacker is a rival with a given student
bool checkRival(Hacker hacker, Student student)
{
    for(int i = 0; i < hacker->m_rivalsSize; i++)
    {
        if(hacker->m_rivals[i] == student)
        {
            return true;
        }
    }

    return false;
}

//friendship functions
int friendshipFunction1(void* person1, void* person2)
{
    Student person1Student = *(Student*)person1;
    Student person2Student = *(Student*)person2;
    
    int friendship = 0; 

    if(person1Student->m_hacker)
    {
        if(checkFriend(person1Student->m_hacker, person2Student))
        {
            friendship = 20;//friendship amount to return
        }
        else if(checkRival(person1Student->m_hacker, person2Student))
        {
            friendship = -20;//rivalry amount to return
        }//otherwise stay at 0
    }
    if(person2Student->m_hacker)
    {
        if(checkFriend(person2Student->m_hacker, person1Student))
        {
            friendship = 20;//friendship amount to return
        }
        else if(checkRival(person2Student->m_hacker, person1Student))
        {
            friendship = -20;//rivalry amount to return
        }//otherwise stay at 0
    }

    return friendship;
}

int friendshipFunction2(void* person1, void* person2)
{
    Student person1Student = *(Student*)person1;
    Student person2Student = *(Student*)person2;

    int nameDiff = 0;

    for(int i = 0; i < MAX(strlen(person1Student->m_name), strlen(person2Student->m_name)); i++)
    {
        if(i >= strlen(person1Student->m_name))
        {
            nameDiff += person2Student->m_name[i];
        }
        else if(i >= strlen(person2Student->m_name))
        {
            nameDiff += person1Student->m_name[i];
        }
        else
        {
            nameDiff += abs(person1Student->m_name[i] - person2Student->m_name[i]);
        }
    }

    for(int i = 0; i < MAX(strlen(person1Student->m_surname), strlen(person2Student->m_surname)); i++)
    {
        if(i >= strlen(person1Student->m_surname))
        {
            nameDiff += person2Student->m_surname[i];
        }
        else if(i >= strlen(person2Student->m_surname))
        {
            nameDiff += person1Student->m_surname[i];
        }
        else
        {
            nameDiff += abs(person1Student->m_surname[i] - person2Student->m_surname[i]);
        }
    }

    return nameDiff;
}

int friendshipFunction3(void* person1, void* person2)
{
    Student person1Student = *(Student*)person1;
    Student person2Student = *(Student*)person2;

    return abs(person1Student->m_ID - person2Student->m_ID);
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
        course = getCourseFromNum(sys, courseNum);
        for(int j = 1; j < countElementsInLine(buffer); j++)
        {
            memcpy(tempBuffer, buffer + j * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
            course->m_queue = IsraeliQueueEnqueue(course->m_queue, getStudentFromID(sys, tempBuffer));
        }
    }

    return sys;
}

void hackEnrollment(EnrollmentSystem sys, FILE* out)
{
    for(int i = 0; i < sys->m_coursesSize; i++)
    {
        sys->m_courses[i]->m_queue = IsraeliQueueAddFriendshipMeasure(sys->m_courses[i]->m_queue, friendshipFunction1);
        sys->m_courses[i]->m_queue = IsraeliQueueAddFriendshipMeasure(sys->m_courses[i]->m_queue, friendshipFunction2);
        sys->m_courses[i]->m_queue = IsraeliQueueAddFriendshipMeasure(sys->m_courses[i]->m_queue, friendshipFunction3);
    }

    for(int i = 0; i < sys->m_hackersSize; i++)
    {
        for(int j = 0; j < sys->m_hackers[i]->m_coursesSize; j++)
        {
            (sys->m_hackers[i]->m_courses[j])->m_queue = IsraeliQueueEnqueue((sys->m_hackers[i]->m_courses[j])->m_queue, sys->m_hackers[i]);
        }
    }

    int coursesDeclined = 0;
    bool success = true;
    IsraeliQueue tempQueue = { 0 };
    for(int i = 0; i < sys->m_hackersSize; i++)
    {
        if(sys->m_hackers[i]->m_coursesSize == 1)
        {
            success = isInCourse(sys->m_hackers[i], sys->m_hackers[i]->m_courses[0]) ? success : false;
        }
        else
        {
            for(int j = 0; j < sys->m_hackers[i]->m_coursesSize; j++)
            {
                coursesDeclined = isInCourse(sys->m_hackers[i], sys->m_hackers[i]->m_courses[j]) ? coursesDeclined : coursesDeclined + 1;
            }

            success = coursesDeclined < 2;
        }
        
        if(!success)
        {
            fprintf(out, "Cannot satisfy constraints for %s", sys->m_hackers[i]->m_student->m_ID);
        }
        coursesDeclined = 0;
    }

    for(int i = 0; i < sys->m_coursesSize; i++)
    {
        fprintf(out, "%d", sys->m_courses[i]->m_number);
        tempQueue = IsraeliQueueClone(sys->m_courses[i]->m_queue);
        while(tempQueue)
        {
            fprintf(out, " %s", (*(Student*)IsraeliQueueDequeue(tempQueue))->m_ID);
        }
        fprintf(out, "\n");
    }
}

void destroyEnrollment(EnrollmentSystem enrollment) {
    int i = 0;
    
    // Do nothing for null.
    if (enrollment == NULL) {
        return;
    }

    for (i = 0; i < enrollment->m_studentsSize; i++) {
        destroyStudent(enrollment->m_students[i]);
    }
    free(enrollment->m_students);

    for (i = 0; i < enrollment->m_coursesSize; i++) {
        destroyCourse(enrollment->m_courses[i]);
    }
    free(enrollment->m_courses);

    for (i = 0; i < enrollment->m_hackersSize; i++) {
        destroyHacker(enrollment->m_hackers[i]);
    }
    free(enrollment->m_hackers);

    // It's good practice to NULL dangling pointers.
    enrollment->m_students = NULL;
    enrollment->m_courses = NULL;
    enrollment->m_hackers = NULL;

    free(enrollment);
}
