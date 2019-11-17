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

#define ARRAY_SIZE 10
#define SECTION_SIZE 5
#define NOT_FOUND 251

//#define SINGLE_THREAD
#define MULTITHREAD
//#define MULTIPROCESS

#if defined SINGLE_THREAD
#define MY_SEARCH search_single_thread
#elif defined MULTITHREAD
#define MY_SEARCH search_multithread
#define GET_RESULTS get_results_mt
#elif defined MULTIPROCESS
#define MY_SEARCH search_multiprocess
#define GET_RESULTS get_results_mp
#endif

extern int myArray[ARRAY_SIZE];
unsigned char search_single_thread(int section, int value);
int section_first_index(int section);
unsigned char get_results_mt(int section);
unsigned char get_results_mp(int section);

#endif /* MULTITEST_H */

