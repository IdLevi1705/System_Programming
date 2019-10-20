
/* 
 *  File:     mymalloc.h
 *  Authors:  Alexander Varshavsky
 *            Idan Levi
 */

#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <stdio.h>
#include <stdlib.h>

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

