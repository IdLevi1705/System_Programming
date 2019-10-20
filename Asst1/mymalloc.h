/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   mymalloc.h
 * Author:
 *
 * Created on October 10, 2019, 10:33 PM
 */

 #include <stdio.h>
 #include <stdlib.h>

#ifndef MYMALLOC_H
#define MYMALLOC_H



#define malloc( x ) mymalloc( x, __FILE__, __LINE__ )
#define free( x ) myfree( x, __FILE__, __LINE__ )

#define FREE    0
#define IN_USE  1

#define DEBUG_MODE



// max available memblock size is 0x7fff (15 bit unsigned integer)

#define MEM_BLOCK_SIZE 4096



static char mymemblock[MEM_BLOCK_SIZE];
static int initialized = 0;

struct Meta
{
    unsigned short available_size  : 15;
    unsigned short inuse : 1;
};

void *mymalloc(size_t, char *, int);
void myfree(void *, char *, int);

int integrithy_check();

#endif /* MYMALLOC_H */
