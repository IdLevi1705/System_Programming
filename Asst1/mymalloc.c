/*
 *  File:     mymalloc.c
 *  Authors:  Alexander Varshavsky
 *            Idan Levi
 */


#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

static char mymemblock[MEM_BLOCK_SIZE];
static int initialized = 0;


void *mymalloc(size_t requested_size, char * file, int line) {

    if (requested_size == 0) {
        printf("[%s]: ERROR in line:%d ---> Requested size is 0 \n", file, line);
        return 0;
    }

    struct Meta *meta_ptr = (struct Meta*) mymemblock;

    if (initialized == 0) {
        initialized = 1;
        meta_ptr->inuse = FREE;
        meta_ptr->available_size = (MEM_BLOCK_SIZE) - sizeof (struct Meta);
    }

    // Check if we can handle the requested mem size
    if (requested_size > (MEM_BLOCK_SIZE) - sizeof (struct Meta)) {
        printf("[%s]: ERROR in line:%d ---> Requested size is too big. Can't fit in memory.\n", file, line);
        return NULL;
    }

    //Find available block
    while (meta_ptr->available_size < requested_size || meta_ptr->inuse == IN_USE) {
        // Jump to the next meta block if available
        meta_ptr = (struct Meta*) (((char *) (meta_ptr)) + meta_ptr->available_size + sizeof (struct Meta));

        //Check if reached the end of the myblock
        if (meta_ptr > (struct Meta*) (&mymemblock[MEM_BLOCK_SIZE] - sizeof (struct Meta) - 1)) {

            // We reached the end of the buffer. No memory is available.
            printf("[%s]: ERROR in line:%d ---> We reached the end of the buffer. No memory is available\n", file, line);
            return NULL;
        }
    }
    //Looks like we found the needed block
    //Init next Meta Block
    //debugging:
    //printf("Next meta:\t%p, inUse: %u, size: %u\n", meta_ptr, meta_ptr->inuse, meta_ptr->available_size);
    if (meta_ptr->available_size >= (requested_size + sizeof (struct Meta) + 1)) {

        struct Meta * meta_free = (struct Meta*) (((char *) (meta_ptr)) + requested_size + sizeof (struct Meta));

        meta_free->inuse = FREE;
        meta_free->available_size = meta_ptr->available_size - requested_size - sizeof (struct Meta);
        //debugging:
        // printf("Meta Free:\t%p, inUse: %u, size: %u\n", meta_free, meta_free->inuse, meta_free->available_size);
    } else {
        requested_size = meta_ptr->available_size;
    }

    meta_ptr->available_size = requested_size;
    meta_ptr->inuse = IN_USE;
    //debugging:
    // printf("Returning meta:\t%p, inUse: %u, size: %u\n\n", meta_ptr, meta_ptr->inuse, meta_ptr->available_size);
    return meta_ptr + 1;
}

void myfree(void *ptr, char * file, int line) {

    //Check if ptr is in the correct range
    if (!(ptr >= (void *) (mymemblock + sizeof (struct Meta)) && ptr <= (void *) (mymemblock + MEM_BLOCK_SIZE - 1))) {
        printf("[%s]: ERROR in line:%d --->  The free request is not valid. address not in range: [%p]\n", file, line, ptr);
        return;
    }

    // Search if pointer is valid by jumping from the beginning + checking if status is already FREE

    //new checker ptr to find all the valid pointers
    struct Meta *checker_ptr = (struct Meta*) mymemblock;
    while (checker_ptr <= (struct Meta *) (mymemblock + MEM_BLOCK_SIZE - 1)) {

        //Checking if the requested ptr is matching any of the valid ptrs
        if ((ptr - sizeof (struct Meta)) != checker_ptr) {
            checker_ptr = (struct Meta*) (((char *) (checker_ptr)) + checker_ptr->available_size + sizeof (struct Meta));
            continue;
        }

        // check if the ptr is alreadt free
        if (checker_ptr->inuse == IN_USE) {
            checker_ptr->inuse = FREE;
            //Debugging:
            //printf("checker pointer in address  %p  is now Free, inUse: %u, size: %u\n\n", checker_ptr, checker_ptr->inuse, checker_ptr->available_size);
            return;
        } else {
            printf("[%s]: ERROR in line:%d ---> The status of [%p] is already FREE \n", file, line, checker_ptr);
            return;
        }
    }

    printf("[%s]: ERROR in line:%d ---> Free request can't be executed. ptr: [%p] \n", file, line, checker_ptr);
    return;
}
