/*
 *  File:     memgrind.c
 *  Authors:  Alexander Varshavsky
 *            Idan Levi
 */


#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

typedef unsigned long int uint64_t;

//GLOBAL VARIABLES
//clock_t start_t, end_t, total_t;

uint64_t start_t, end_t, total_t;

#define REPEATE 150   // # of total mallocs for test B.
#define RUNS 100      // # of total repeats of each test.

// Total time for each test. Number of runs set in global RUNS.
uint64_t totalTime_test_A = 0.0;
uint64_t totalTime_test_B = 0.0;
uint64_t totalTime_test_C = 0.0;
uint64_t totalTime_test_D = 0.0;
uint64_t totalTime_test_E = 0.0;
uint64_t totalTime_test_F = 0.0;

// Total runs per test.
int totalRuns_test_A = 0;
int totalRuns_test_B = 0;
int totalRuns_test_C = 0;
int totalRuns_test_D = 0;
int totalRuns_test_E = 0;
int totalRuns_test_F = 0;


// returns formatter time in microseconds.
uint64_t mygettime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * (uint64_t) 1000000) + (time.tv_usec);
}

//malloc() 1 byte and immediately free it - do this 150 times

void test_A() {
    int i;
    void * ptr;
    uint64_t runningTime = 0.0;

    start_t = mygettime();

    for (i = 0; i < 150; i++) {
        ptr = malloc(1);
        free(ptr);
    }
    end_t = mygettime();
    runningTime = end_t - start_t;
    totalRuns_test_A++;
    totalTime_test_A = totalTime_test_A + runningTime;
}

//malloc() 1 byte, store the pointer in an array - do this 150 times.
// Once you've malloc()ed 50 byte chunks, then free() the 50 1 byte pointers one by one.

void test_B() {
    int i, j;
    void * ptr[REPEATE];
    uint64_t runningTime = 0;
    start_t = mygettime();

    for (j = 0; j < 3; j++) {
        for (i = 0; i < 50; i++) {
            ptr[i] = malloc(1);
        }
        for (i = 0; i < 50; i++) {
            free(ptr[i]);
        }
    }
    end_t = mygettime();
    runningTime = (end_t - start_t);
    totalRuns_test_B++;
    totalTime_test_B = totalTime_test_B + runningTime;
}

// Randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer
// do this until you have allocated 50 times. After allocating 50 times, keep freeing until all pointers free.

void test_C() {

    int i, lastArrId = 0, counterFree = 0;
    char *arr[50];
    uint64_t runningTime = 0;
    int mallocCnt = 0;
    void *ptr;

    //start time
    start_t = mygettime();
    //generate random number | different number each time.
    srand(time(0));

    while (mallocCnt < 50) {
        int random = rand() % 2;

        //random numbers 0 for malloc 1 for free
        if (random == 0) {
            mallocCnt++;
            ptr = malloc(1);
            arr[lastArrId] = ptr;
            lastArrId++;
        } else if (random == 1) {
            if (lastArrId != 0) {
                lastArrId--;
            }
            free(arr[lastArrId]);
            counterFree++;
        }
    }

    for (i = 0; i < lastArrId; i++) {
        free(arr[i]);
        counterFree++;
    }
    //end of the process end of time.
    end_t = mygettime();
    runningTime = (end_t - start_t);
    totalRuns_test_C++;
    totalTime_test_C = totalTime_test_C + runningTime;
}

/*
 Randomly choose between a randomly-sized malloc() or free()ing a pointer – do this many
 times (see below)
- Keep track of each malloc so that all mallocs do not exceed your total memory capacity
- Keep track of each operation so that you eventually malloc() 50 times
- Keep track of each operation so that you eventually free() all pointers
- Choose a random allocation size between 1 and 64 bytes
 */
void test_D() {

    //random numbers 0 for malloc 1 for free
    int i, lastArrId = 0, counterFree = 0;
    char *arr[50];
    uint64_t runningTime = 0;
    int mallocCnt = 0;
    void *ptr;

    //start time
    start_t = mygettime();
    //generate random number | different number each time.
    srand(time(0));

    while (mallocCnt < 50) {
        int random = rand() % 2;

        if (random == 0) {
            mallocCnt++;
            int rand_size_1_to_64 = (rand() % 64 + 1);
            ptr = malloc(rand_size_1_to_64);
            arr[lastArrId] = ptr;
            lastArrId++;
        } else if (random == 1) {
            if (lastArrId != 0) {
                lastArrId--;
            }
            free(arr[lastArrId]);
            counterFree++;
        }
    }

    for (i = 0; i < lastArrId; i++) {
        free(arr[i]);
        counterFree++;
    }
    //end of the proccess end of time.
    end_t = mygettime();
    runningTime = (end_t - start_t);
    totalRuns_test_D++;
    totalTime_test_D = totalTime_test_D + runningTime;
}

// runs malloc() 50 times, each time allocating a random size between 85 to 150. Total memory is 4096 which means
// that we will run out of memory for sure as 85*50 is already more than what we have available.
// after runing out of memory, free all the pointers.

void test_E() {

    int i, lastArrId = 0, counterFree = 0;
    char *arr[50];
    uint64_t runningTime = 0;
    int mallocCnt = 0;
    void *ptr;

    //start time
    start_t = mygettime();
    //generate random number | different number each time.
    srand(time(0));

    while (mallocCnt < 50) {
        mallocCnt++;
        //random number between 85 - 150
        int rand_size_85_to_150 = (rand() % 65 + 85);
        ptr = malloc(rand_size_85_to_150);
        if (ptr != 0) {
            arr[lastArrId] = ptr;
            lastArrId++;
        }
    }

    for (i = 0; i < lastArrId; i++) {
        free(arr[i]);
        counterFree++;
    }
    //end of the proccess end of time.
    end_t = mygettime();
    runningTime = (end_t - start_t);
    totalRuns_test_E++;
    totalTime_test_E = totalTime_test_E + runningTime;
}

// allocate once, random size between 0 and 8000. 
// then try to free pointer 10 times.

void test_F() {

    int i;
    uint64_t runningTime = 0;
    void *ptr;

    //start time
    start_t = mygettime();
    //generate random number | different number each time.
    srand(time(0));

    int rand_size_0_to_8000 = (rand() % 8001);
    ptr = malloc(rand_size_0_to_8000);

    for (i = 0; i < 10; i++) {
        free(ptr);
    }


    //end of the proccess end of time.
    end_t = mygettime();
    runningTime = (end_t - start_t);
    totalRuns_test_F++;
    totalTime_test_F = totalTime_test_F + runningTime;
}

int main(int argc, char** argv) {

    // Runs test A-F. Repeats RUNS times.
    int i = 0;
    for (; i < RUNS; i++) {
        printf("\n****************************************************************************\n");
        printf("****************************** Running test A ******************************\n");
        printf("****************************************************************************\n\n");
        test_A();

        printf("\n****************************************************************************\n");
        printf("****************************** Running test B ******************************\n");
        printf("****************************************************************************\n\n");
        test_B();

        printf("\n****************************************************************************\n");
        printf("****************************** Running test C ******************************\n");
        printf("****************************************************************************\n\n");
        test_C();

        printf("\n****************************************************************************\n");
        printf("****************************** Running test D ******************************\n");
        printf("****************************************************************************\n\n");
        test_D();

        printf("\n****************************************************************************\n");
        printf("****************************** Running test E ******************************\n");
        printf("****************************************************************************\n\n");
        test_E();

        printf("\n****************************************************************************\n");
        printf("****************************** Running test F ******************************\n");
        printf("****************************************************************************\n\n");
        test_F();
    }
    // print results
    printf("\n****************************************************************************\n");
    printf("***************************** Summary Test A-F *****************************\n");
    printf("****************************************************************************\n\n");
    printf("Total runs for Test A: %d\nAverage run time for test A is: %0.9lf milliseconds\n\n", totalRuns_test_A, (totalTime_test_A / totalRuns_test_A) / 1000.0);
    printf("Total runs for Test B: %d\nAverage run time for test B is: %0.9lf milliseconds\n\n", totalRuns_test_B, (totalTime_test_B / totalRuns_test_B) / 1000.0);
    printf("Total runs for Test C: %d\nAverage run time for test C is: %0.9lf milliseconds\n\n", totalRuns_test_C, (totalTime_test_C / totalRuns_test_C) / 1000.0);
    printf("Total runs for Test D: %d\nAverage run time for test D is: %0.9lf milliseconds\n\n", totalRuns_test_D, (totalTime_test_D / totalRuns_test_D) / 1000.0);
    printf("Total runs for Test E: %d\nAverage run time for test E is: %0.9lf milliseconds\n\n", totalRuns_test_E, (totalTime_test_E / totalRuns_test_E) / 1000.0);
    printf("Total runs for Test F: %d\nAverage run time for test F is: %0.9lf milliseconds\n\n", totalRuns_test_F, (totalTime_test_F / totalRuns_test_F) / 1000.0);
    return (0);
}
