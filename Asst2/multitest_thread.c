/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <math.h>
#include "multitest.h"
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h>

struct data {
    int section;
    int value;
};

pthread_t myIdArray[(ARRAY_SIZE / SECTION_SIZE) + 1];
struct data request_ar[(ARRAY_SIZE / SECTION_SIZE) + 1];

void *search_func(void *req) {
    struct data *request = (struct data *) req;
    sleep(1);
    printf("Printing from Thread %d - %d \n", request->section, request->value);
    unsigned char result = search_single_thread(request->section, request->value);
    pthread_exit((void*) result);
    return NULL;
}

unsigned char search_multithread(int section, int value) {
    request_ar[section].section = section;
    request_ar[section].value = value;
    printf("Printing from search %d - %d \n", request_ar[section].section, request_ar[section].value);
    pthread_create(&myIdArray[section], NULL, search_func, &request_ar[section]);
    sleep(1);
    return NOT_FOUND;
}

unsigned char get_results_mt(int section) {

    unsigned char * result = NULL;
    pthread_join(myIdArray[section], &result);
    // printf("get_results_mt -> %03u\n", (unsigned int) result);
    return result;


}