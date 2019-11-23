/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "multitest.h"

struct data
{
    int section;
    int value;
    struct test_cases my_cases;
};

struct thread
{
    pthread_t id;
    int checked;
};

struct thread * myIdArray;
struct data * request_ar;

void *search_func(void *req)
{
    struct data *request = (struct data *) req;
    int result = search_single_thread(request->section, request->value, &request->my_cases);
    pthread_exit((void*) result);
    return NULL;
}

//searches using multithread
int search_multithread(int *array, struct test_cases * my_cases, int total_sections, int search_value)
{
    if (myIdArray != NULL) {
        free(myIdArray);
    }
    if (request_ar != NULL) {
        free(request_ar);
    }

    myIdArray = (pthread_t *) malloc(total_sections * sizeof (struct thread));
    request_ar = (struct data *) malloc(total_sections * sizeof (struct data));

    memset(myIdArray, 0, total_sections * sizeof (struct thread));
    memset(request_ar, 0, total_sections * sizeof (struct data));

    int section = 0;
    for (; section < total_sections; section++) {
        request_ar[section].section = section;
        request_ar[section].value = search_value;
        request_ar[section].my_cases = *my_cases;
        pthread_create(&myIdArray[section].id, NULL, search_func, &request_ar[section]);

    }
    return get_results_mt(total_sections, my_cases->section_size);
}

int get_results_mt(int total_sections, int section_size)
{
    int fin_result = -1;
    int section = 0;
    void * result = NULL;
    for (; section < total_sections; section++) {

        if (myIdArray[section].checked == 0) {
            pthread_join(myIdArray[section].id, (void **) &result);
            myIdArray[section].checked = 1;
        }
        if ((int) result != NOT_FOUND) {
            fin_result = (int) result + section_first_index(section, section_size);
            printf("final result:  -> %d\n", fin_result);
        }
    }
    return fin_result;
}
