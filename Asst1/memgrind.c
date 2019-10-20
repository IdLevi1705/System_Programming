/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
//#include <stdlib.h>

//GLOBAL VARIABLES
clock_t start_t, end_t, total_t;

#define REPEATE 150

double totalTime_test_A = 0.0;
double totalTime_test_B = 0.0;
double totalTime_test_C = 0.0;
double totalTime_test_D = 0.0;
double totalTime_test_E = 0.0;
double totalTime_test_F = 0.0;

int totalRuns_test_A = 0;
int totalRuns_test_B = 0;
int totalRuns_test_C = 0;
int totalRuns_test_D = 0;
int totalRuns_test_E = 0;
int totalRuns_test_F = 0;

void test_A() {
    int i;
    void * ptr;
    double runningTime = 0.0;

    start_t = clock();

    for (i = 0; i < 15; i++) {
        ptr = malloc(1);
        free(ptr);
    }

    end_t = clock();
    runningTime = (double) (end_t - start_t) / CLOCKS_PER_SEC;
    totalRuns_test_A++;

    // printf("Run #%d of Test_A took: %0.9f seconds\n", totalRuns_test_A, runningTime);
    totalTime_test_A = totalTime_test_A + runningTime;
    //  printf("Total time for test A is: %0.9f\nTotal runs: %d\n\n", totalTime_test_A, totalRuns_test_A);



}

void test_B() {
    int i, j;
    void * ptr[REPEATE];
    double runningTime = 0.0;

    start_t = clock();

    for (j = 3; j != 0; j--) {
        //  printf("+++++++++++++++++\n");
        for (i = 0; i < REPEATE; i++) {
            ptr[i] = malloc(j + 1);
        }
        //   printf("----------------\n");
        for (i = 0; i < REPEATE; i++) {
            free(ptr[i]);
        }
    }

    end_t = clock();
    runningTime = (double) (end_t - start_t) / CLOCKS_PER_SEC;
    totalRuns_test_B++;

    //   printf("Run #%d of Test_A took: %0.9f seconds\n", totalRuns_test_B, runningTime);
    totalTime_test_B = totalTime_test_B + runningTime;
    //  printf("Total time for test A is: %0.9f\nTotal runs: %d\n\n", totalTime_test_B, totalRuns_test_B);
}

void test_C() {

    //random numbers 0 for malloc 1 for free
    int i, lastArrId = 0, counterFree = 0;
    char *arr[50];
    double runningTime = 0.0;
    int mallocCnt = 0;
    void *ptr;

    //start time
    start_t = clock();
    //generate random number | different number each time.
    srand(time(0));

    while (mallocCnt < 50) {
        int random = rand() % 2;

        //     printf("random %d and sleep\n", random);
        //  usleep(250);

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

    // printf("Total malloc: %d lastArrId: %d\n", mallocCnt, lastArrId);
    //  printf("Total free: %d \n", counterFree);

    for (i = 0; i < lastArrId; i++) {
        free(arr[i]);
        counterFree++;
    }
    //end of the proccess end of time.
    end_t = clock();
    runningTime = (double) (end_t - start_t) / CLOCKS_PER_SEC;
    totalRuns_test_C++;

    // printf("Run #%d of Test_A took: %0.9f seconds\n", totalRuns_test_C, runningTime);
    totalTime_test_C = totalTime_test_C + runningTime;
    //  printf("Total time for test A is: %0.9f\nTotal runs: %d\n\n", totalTime_test_C, totalRuns_test_C);
    // printf("%d\n", counterAllocate);

    //   printf("Total malloc: %d \n", mallocCnt);
    //  printf("Total free: %d \n", counterFree);
}

int main(int argc, char** argv) {


    for (int i = 0; i < 100; i++) {
        test_A();
    }
    printf("Average run time for test A is: %0.9lf\nTotal runs: %d\n", totalTime_test_A / totalRuns_test_A, totalRuns_test_A);

    for (int i = 0; i < 100; i++) {
        test_B();
    }
    printf("Average run time for test B is: %0.9lf\nTotal runs: %d\n", totalTime_test_B / totalRuns_test_B, totalRuns_test_B);

    for (int i = 0; i < 100; i++) {
        test_C();
    }
    printf("Average run time for test C is: %0.9lf\nTotal runs: %d\n", totalTime_test_C / totalRuns_test_C, totalRuns_test_C);
    //    test_D();




    return (0);
}
