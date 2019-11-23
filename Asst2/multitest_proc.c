/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "multitest.h"


extern pid_t parent_pid;

int search_multiprocess(int *array, struct test_cases * my_cases, int total_sections, int search_value)
{
    if (parent_pid == 0) {
        parent_pid = getpid();
    }

    int section = 0;
    pid_t cpid = 0;
    int fin_result = -1;
    unsigned char result = NOT_FOUND;
    for (; section < total_sections; section++) {
        if (parent_pid == getpid())
            cpid = fork();
        else
            return 0;
        //######################
        //##### PARENT PROCESS #####
        //######################

        if (cpid > 0) {
            int status;
            wait(&status);

            if (WEXITSTATUS(status) == 251) {
            } else if (WEXITSTATUS(status) == -1) {
                printf("Returned to parent: %d -> ERROR: child wasn't found.\n", WEXITSTATUS(status));
            } else {
                fin_result = WEXITSTATUS(status);
                fin_result = fin_result + section_first_index(section, my_cases->section_size);
            }
            //#####################
            // ##### CHILD PROCESS #####
            //#####################            
        } else if (cpid == 0) {

            result = search_single_thread(section, search_value, my_cases);
            exit(result);
        } else {
            printf("Error in forking..\n");
            exit(EXIT_FAILURE);
        }

    }

    return fin_result;
}

