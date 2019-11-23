/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   multitest.h
 * Author: alex
 *
 * Created on November 14, 2019, 9:18 PM
 */

#ifndef MULTITEST_H
#define MULTITEST_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h> // for wait() 
#include <unistd.h> // for fork()
#include <sys/time.h>

#define NOT_FOUND 251
#define SEARCH_VALUE 5

//#define SINGLE_THREAD
//#define MULTITHREAD
#define MULTIPROCESS


#if !defined(MULTITHREAD) && !defined(MULTIPROCESS)
#error "Define MULTITHREAD  or MULTIPROCESS"
#endif


#if defined SINGLE_THREAD
#define MY_SEARCH search_single_thread
#define CUR_SECTION_INIT 1
#elif defined MULTITHREAD
#define MODE "MULTITHREAD"
#define MY_SEARCH search_multithread
#define CUR_SECTION_INIT 1
#define GET_RESULTS get_results_mt
#elif defined MULTIPROCESS
#define MODE "MULTIPROCESS"
#define MY_SEARCH search_multiprocess
#define CUR_SECTION_INIT 0

#endif
struct test_cases{
    int array_size;
    int section_size;
};

extern int * myArray;
extern int current_section;
extern int * myResults;
unsigned char search_single_thread(int section, int value, struct test_cases * my_cases);
int shuffleIndex(int * p, size_t size, int last_index);
int section_first_index(int section, int section_size);
int get_results_mt(int total_sections, int section_size);
int search_multithread(int *array, struct test_cases *my_cases,  int total_sections, int search_value);
int search_multiprocess(int *array, struct test_cases * my_cases, int total_sections, int search_value);

#endif /* MULTITEST_H */

