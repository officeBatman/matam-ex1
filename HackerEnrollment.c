#include "HackerEnrollment.h"

#include <stdbool.h>
#include <assert.h>

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
    course->m_queue = NULL;

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

char* cloneString(const char* string) {
    if (string == NULL) {
        return NULL;
    }

    unsigned int len = strlen(string);
    char* out = (char*)malloc(sizeof(char) * (len + 1));
    if (out == NULL) {
        return NULL;
    }

    strcpy(out, string);
    return out;
}

Student createStudent(char ID[ID_SIZE + 1], unsigned int credits, unsigned int GPA, char* name, char* surname, char* city, char* department, Hacker hacker) {
    Student out = (Student)malloc(sizeof(struct Student_t));
    if(out == NULL)
    {
        return NULL;
    }

    memcpy(out->m_ID, ID, ID_SIZE + 1);
    out->m_credits = credits;
    out->m_GPA = GPA;
    out->m_name = cloneString(name);
    out->m_surname = cloneString(surname);
    out->m_city = cloneString(city);
    out->m_department = cloneString(department);
    out->m_hacker = hacker;

    if (!out->m_name || !out->m_surname || !out->m_city || !out->m_department) {
        destroyStudent(out);
        out = NULL;
    }

    return out;
}

//parses the students file and saves the information
Student* parseStudentsFile(FILE* studentsFile, int* studentsSize)
{
    int i = 0;
    bool error = false;
    char mainBuffer[BUFFER_SIZE + 1] = { 0 };
    char IDBuffer[ID_SIZE + 1] = { 0 };
    unsigned int credits = 0;
    unsigned int GPA = 0;
    char nameBuffer[BUFFER_SIZE + 1] = { 0 };
    char surnameBuffer[BUFFER_SIZE + 1] = { 0 };
    char cityBuffer[BUFFER_SIZE + 1] = { 0 };
    char departmentBuffer[BUFFER_SIZE + 1] = { 0 };

    int studentsAmount = getLineNum(studentsFile);
    Student* students = (Student*)malloc(sizeof(Student) * studentsAmount);

    if (!students) {
        return NULL;
    }

    while(fgets(mainBuffer, BUFFER_SIZE, studentsFile)) {
        sscanf(
            mainBuffer,
            "%s %d %d %s %s %s %s",
            IDBuffer, &credits, &GPA,
            nameBuffer, surnameBuffer, cityBuffer, departmentBuffer
        );

        students[i] = createStudent(
            IDBuffer, credits, GPA,
            nameBuffer, surnameBuffer, cityBuffer, departmentBuffer,
            NULL
        );
        error = !students[i] ? true : error;
        i++;
    }

    if (error) {
        for (i = 0; i < studentsAmount; i++) {
            destroyStudent(students[i]);
        }
        free(students);
        return NULL;
    }

    *studentsSize = studentsAmount;
    return students;
}

Course createCourse(unsigned int number, unsigned int size) {
    FriendshipFunction emptyFriendships[1] = { NULL };

    Course out = (Course)malloc(sizeof(struct Course_t));
    if (!out) {
        return NULL;
    }

    out->m_number = number;
    out->m_size = size;
    out->m_queue = IsraeliQueueCreate(emptyFriendships, NULL, FRIENDSHIP_THRESHOLD, RIVALRY_THRESHOLD);

    if (!out->m_queue) {
        destroyCourse(out);
        out = NULL;
    }

    return out;
}

//parses the courses file and saves the information
Course* parseCoursesFile(FILE* coursesFile, int* coursesSize)
{
    char buffer[BUFFER_SIZE + 1] = { 0 };
    int i = 0;
    bool error = false;
    unsigned int number = 0;
    unsigned int size = 0;

    int coursesAmount = getLineNum(coursesFile);
    Course* courses = (Course*)malloc(sizeof(Course) * coursesAmount);
    if(!courses)
    {
        return NULL;
    }

    while(fgets(buffer, BUFFER_SIZE, coursesFile))
    {
        sscanf(buffer, "%d %d", &number, &size);
        courses[i] = createCourse(number, size);
        error = !courses[i] ? true : error;
        i++;
    }

    if (error) {
        for (i = 0; i < coursesAmount; i++) {
            destroyCourse(courses[i]);
        }
        free(courses);
        return NULL;
    }

    *coursesSize = coursesAmount;
    return courses;
}

// Creates a hacker associated with a student instance. Sets the student's
// hacker pointer to the created hacker.
Hacker createHacker(Student student, int courses, int friends, int rivals) {
    assert(student);
    assert(student->m_hacker == NULL);

    Hacker out = (Hacker)malloc(sizeof(struct Hacker_t));
    if (!out) {
        return NULL;
    }

    out->m_student = student;
    out->m_student->m_hacker = out;
    out->m_coursesSize = courses;
    out->m_friendsSize = friends;
    out->m_rivalsSize = rivals;

    out->m_courses = (Course*)malloc(sizeof(Course) * courses);
    out->m_friends = (Student*)malloc(sizeof(Student) * friends);
    out->m_rivals = (Student*)malloc(sizeof(Student) * rivals);

    if (!out->m_courses || !out->m_friends || !out->m_rivals) {
        destroyHacker(out);
        out = NULL;
    }

    return out;
}

Hacker parseHacker(EnrollmentSystem sys, char* IDBuffer, char* coursesBuffer, char* friendsBuffer, char* rivalsBuffer) {
    Hacker hacker = NULL;
    char tempBuffer[ID_SIZE + 1] = { 0 };
    Student student = NULL;
    int courses = 0;
    int friends = 0;
    int rivals = 0;
    int i = 0;

    sscanf(IDBuffer, "%s", tempBuffer);
    student = getStudentFromID(sys, tempBuffer);

    courses = countElementsInLine(coursesBuffer);
    friends = countElementsInLine(friendsBuffer);
    rivals = countElementsInLine(rivalsBuffer);

    hacker = createHacker(student, courses, friends, rivals);
    if (!hacker) {
        return NULL;
    }

    for(i = 0; i < courses; i++) {
        memcpy(tempBuffer, coursesBuffer + i * (COURSE_NUM_LEN + 1), COURSE_NUM_LEN);
        hacker->m_courses[i] = getCourseFromNum(sys, atoi(tempBuffer));
    }

    for(i = 0; i < friends; i++) {
        memcpy(tempBuffer, friendsBuffer + i * (ID_SIZE + 1), ID_SIZE);
        hacker->m_friends[i] = getStudentFromID(sys, tempBuffer);
    }

    for(i = 0; i < rivals; i++) {
        memcpy(tempBuffer, rivalsBuffer + i * (ID_SIZE + 1), ID_SIZE);
        hacker->m_rivals[i] = getStudentFromID(sys, tempBuffer);
    }

    return hacker;
}

//parses the hackers file and saves the information
Hacker* parseHackersFile(EnrollmentSystem sys, FILE* hackersFile, int* hackersSize)
{
    bool error = false;
    char IDBuffer[ID_SIZE + 1] = { 0 };
    char coursesBuffer[BUFFER_SIZE + 1] = { 0 };
    char friendsBuffer[BUFFER_SIZE + 1] = { 0 };
    char rivalsBuffer[BUFFER_SIZE + 1] = { 0 };
    char tempBuffer[ID_SIZE + 1] = { 0 };

    int hackersAmount = getLineNum(hackersFile) / 4;
    Hacker* hackers = (Hacker*)malloc(sizeof(Hacker) * hackersAmount);
    if(!hackers) {
        return NULL;
    }

    for(int i = 0; i < hackersAmount; i++) {
        fgets(IDBuffer, BUFFER_SIZE, hackersFile);  
        fgets(coursesBuffer, BUFFER_SIZE, hackersFile);  
        fgets(friendsBuffer, BUFFER_SIZE, hackersFile);  
        fgets(rivalsBuffer, BUFFER_SIZE, hackersFile);  

        hackers[i] = parseHacker(sys, IDBuffer, coursesBuffer, friendsBuffer, rivalsBuffer);
        error = !hackers[i] ? true : error;
    }

    if (error) {
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

int stringDiff(const char* str1, const char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int len = MAX(len1, len2);
    int sum = 0;

    for(int i = 0; i < len; i++)
    {
        if(i >= len1)
        {
            sum += str2[i];
        }
        else if(i >= len2)
        {
            sum += str1[i];
        }
        else
        {
            sum += abs(str1[i] - str2[i]);
        }
    }

    return sum;
}

int friendshipFunction2(void* person1, void* person2)
{
    Student person1Student = *(Student*)person1;
    Student person2Student = *(Student*)person2;

    int nameDiff = 0;

    nameDiff += stringDiff(person1Student->m_name, person2Student->m_name);
    nameDiff += stringDiff(person1Student->m_surname, person2Student->m_surname);

    return nameDiff;
}

int friendshipFunction3(void* person1, void* person2)
{
    Student person1Student = *(Student*)person1;
    Student person2Student = *(Student*)person2;

    return abs(atoi(person1Student->m_ID) - atoi(person2Student->m_ID));
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
    sys->caseSensitive = false;

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

void setCaseSensitive(EnrollmentSystem sys, bool sensitive) {
    
}
