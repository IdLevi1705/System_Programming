/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   mymalloc.c
 * Author:
 *
 * Created on October 10, 2019, 10:32 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

void *mymalloc(size_t requested_size, char * file, int line) {

    if(requested_size == 0){
        printf("requested size is 0!!! \n");
        return 0;
    }

    struct Meta *meta_ptr = (struct Meta*) mymemblock;


    if (initialized == 0) {
        initialized = 1;
        meta_ptr->inuse = FREE;
        meta_ptr->available_size = (MEM_BLOCK_SIZE) - sizeof (struct Meta);
    }


    /*
     * Check if we can handle the requested mem size
     */
    if (requested_size > (MEM_BLOCK_SIZE) - sizeof (struct Meta))
        return NULL;
    /*
     * Find available block
     */
    while (meta_ptr->available_size < requested_size || meta_ptr->inuse == IN_USE) {
        //        printf("Next meta: %p, inUse: %u, size: %u\n", meta, meta->inuse, meta->available_size);
        /*
         * Jump to the next meta block if available
         */
        meta_ptr = (struct Meta*) (((char *) (meta_ptr)) + meta_ptr->available_size + sizeof (struct Meta));

        /*
         * Check if reached end of the myblock
         */
        if (meta_ptr > (struct Meta*) (&mymemblock[MEM_BLOCK_SIZE] - sizeof (struct Meta) - 1)) {
            /*
             * We reached the end of the buffer.
             * No memory is available
             */
            printf("We reached the end of the buffer. \n No memory is available\n");

            return NULL;
        }
    }
    /*
     * Looks like we found the needed block
     */
 ///////////////////   printf("Next meta:\t%p, inUse: %u, size: %u\n", meta_ptr, meta_ptr->inuse, meta_ptr->available_size);
    /*
     * Init next Meta Block
     */
    if (meta_ptr->available_size >= (requested_size + sizeof(struct Meta) + 1)) {

        struct Meta * meta_free = (struct Meta*) (((char *) (meta_ptr)) + requested_size + sizeof (struct Meta));

        meta_free->inuse = FREE;
        meta_free->available_size = meta_ptr->available_size - requested_size - sizeof (struct Meta);

 ///////////       printf("Meta Free:\t%p, inUse: %u, size: %u\n", meta_free, meta_free->inuse, meta_free->available_size);
    } else {
        requested_size = meta_ptr->available_size;
    }

    meta_ptr->available_size = requested_size;
    meta_ptr->inuse = IN_USE;
//////////////    printf("Returning meta:\t%p, inUse: %u, size: %u\n\n", meta_ptr, meta_ptr->inuse, meta_ptr->available_size);
    return meta_ptr + 1;
}

void myfree(void *ptr, char * file, int line) {

    /*
     * Check if ptr is in the correct range
     */
 ////////////   printf("\n checking if ptr valid...\n");
    if (!(ptr >= (void *) (mymemblock + sizeof (struct Meta)) && ptr <= (void *) (mymemblock + MEM_BLOCK_SIZE - 1))) {
        printf(" ERROR in file: %s  line: %d  -->  the free request is not valid: %p\n", file, line, ptr);
        return;
    }
 ///////////   printf(" ---> %s:  line: %d:  good ptr: %p\n\n", file, line, ptr);

    /*
     * Search if pointer is valid by jumping from the beginning + checking if status is already FREE
     */

    struct Meta *checker_ptr = (struct Meta*) mymemblock;


    while (checker_ptr <= (void *) (mymemblock + MEM_BLOCK_SIZE - 1)) {


        if ((ptr - sizeof (struct Meta)) != checker_ptr) {
            checker_ptr = (struct Meta*) (((char *) (checker_ptr)) + checker_ptr->available_size + sizeof (struct Meta));

            //printf("next checker_ptr: %p\n", checker_ptr);
            continue;
        }

        if (checker_ptr->inuse == IN_USE) {
   /////////         printf("status of %p is IN_USE \n", checker_ptr);
            checker_ptr->inuse = FREE;
   /////////////         printf("checker pointer in address  %p  is Free, inUse: %u, size: %u\n\n", checker_ptr, checker_ptr->inuse, checker_ptr->available_size);
            return;
        } else {
     //       printf("ERROR!!! --> The status of %p is already FREE \n", checker_ptr);
            return;
        }
    }

    printf("pointer is invalid");

    return;
}

//int integrithy_check() {
//    /*
//     * Check if the buffer and meta are not corrupted
//     * by running over meta headers till end.
//     * Do it for every call to malloc and free
//     */
//
//    struct Meta *integrity_pointer = (struct Meta*) mymemblock;
//
//    printf("initial integrity %p \n", integrity_pointer);
//
//    while (integrity_pointer <= (void *) (mymemblock + MEM_BLOCK_SIZE - 1)) {
//
//        integrity_pointer = (struct Meta*) (((char *)(integrity_pointer)) + integrity_pointer->available_size + sizeof (struct Meta));
//        printf("integrity check:  %p \n", integrity_pointer);
//        if ((integrity_pointer->available_size == 0) && integrity_pointer != (void *) (mymemblock + MEM_BLOCK_SIZE - 1)){
//
//            printf("bardakus maximus");
//        } else if ((integrity_pointer->available_size != 0) && integrity_pointer == (void *) (mymemblock + MEM_BLOCK_SIZE - 1)){
//
//            printf("bardakus maximus");
//        }
//
//
//        }
//    printf("all good\n");
//    return 1;
//}

/*
 *
 */
