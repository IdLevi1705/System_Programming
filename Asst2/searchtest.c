
#include <stdio.h>
#include <math.h>
#include "multitest.h"

int myArray[ARRAY_SIZE];

void print_array(int * p, size_t size) {
    int i = 0;
    for (; i < size; i++) {
        printf("index: %d -> %d \n", i, p[i]);
    }
}

int shuffleArray(int * p, size_t size) {
    int i = 0;
    srand(time(0));
    int s = floor(size / 4)*3;
    for (; i < s; i++) {
        int random_a = rand() % size;
        int random_b = rand() % size;
        int temp = 0;
        temp = p[random_a];
        p[random_a] = p[random_b];
        p[random_b] = temp;
        printf("a - %d   b - %d\n", p[random_a], p[random_b]);
    }
    print_array(p, size);
}

int * makeArray(int size) {

    int i = 0;
    for (; i < size; i++) {
        myArray[i] = i + 1;
    }
    print_array(myArray, size);
    shuffleArray(myArray, size);
    return myArray;
}

int section_first_index(int section) {
    return section*SECTION_SIZE;
}

int section_last_index(int section) {
    if (((section + 1) * SECTION_SIZE) < (ARRAY_SIZE)) {
        return (section + 1)*SECTION_SIZE;
    }
    return (ARRAY_SIZE);
}

int get_total_sections() {
    return ceil(ARRAY_SIZE / (double) SECTION_SIZE);

}

unsigned char search_single_thread(int section, int value) {
    int i = section_first_index(section);
    int last_i = section_last_index(section);

    printf("%d --  %d\n", i, last_i);
    for (; i < last_i; i++) {
        if (myArray[i] == value) {
            //printf(" return value: %d \n", (i % SECTION_SIZE));
            return i % SECTION_SIZE;
        }
    }
    return NOT_FOUND;
}

/* main function to call above defined function */
int main() {

    int *p;
    printf(" section #:  %d \n ", get_total_sections());
    p = makeArray(ARRAY_SIZE);


    int i = 0;
    for (; i < get_total_sections(); i++) {
        unsigned char result = MY_SEARCH(i, 5);
#if defined SINGLE_THREAD
        if (result != NOT_FOUND) {
            result = result + section_first_index(i);
            printf("%d\n", result);
            break;
        }
#endif
    }
#ifndef SINGLE_THREAD
    i = 0;
    for (; i < get_total_sections(); i++) {
        unsigned char result = GET_RESULTS(i);
        if (result != NOT_FOUND) {
            int fresult = result + section_first_index(i);
            printf("final result:  ->%d\n", fresult);
            break;
        }
    }
#endif






    sleep(2);
    return 0;
}

//pthread_t tid;
//pthread_create(&tid,....
//        
//pthread_join(tid        