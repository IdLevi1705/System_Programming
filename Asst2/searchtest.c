
//Search value defined in multitest.h

#include "multitest.h"

#define print_array //print_array_ex // uncomment prefious "//" to print array (not recommended)
#define shuffleArray shuffleArray_ex
#define shuffleIndex shuffleIndex_ex


int * myArray = NULL;
int * myResults = NULL;

typedef unsigned long int uint64_t;
// returns formatter time in microseconds.
uint64_t mygettime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * (uint64_t) 1000000) + (time.tv_usec);
}

// prints created array, off by default
void print_array_ex(int * p, size_t size)
{
    int i = 0;
    for (; i < size; i++) {
        printf("index: %d -> %d \n", i, p[i]);
    }
}

//shuffles 75% of the array values
int shuffleArray_ex(int * p, size_t size)
{
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
    }
    print_array(p, size);
}

// shuffles index of the previously found value with random index
void shuffleIndex_ex(int *p, int size, int last_index)
{
    if (last_index == -1) {
        printf("ERROR: I got -1\n");
        return;
    }
   
    printf("\n");
    srand(time(0));
    int a = rand() % size;
    while (a == last_index) {
        a = rand() % size;
    }
    int last = p[last_index];
    p[last_index] = p[a];
    p[a] = last;
    print_array(p, size);
}

// Creates array and fills it with its index+1
int * makeArray(int size, int section_size)
{
    if (myArray != NULL) {
        free(myArray);
    }
    if (myResults != NULL) {
        free(myResults);
    }
    myArray = (int *) malloc(size * sizeof (int));
    myResults = (int *) malloc(((size / section_size) + 1) * sizeof (int));
    int i = 0;
    for (; i < size; i++) {
        myArray[i] = i + 1;
    }
    print_array(myArray, size);

    shuffleArray(myArray, size);
    return myArray;
}

//returns the first index of a section
int section_first_index(int section, int section_size)
{
    return section*section_size;
}
//returns the last index of a section
int section_last_index(int section, struct test_cases * my_cases)
{
    if (((section + 1) * my_cases->section_size) < (my_cases->array_size)) {
        return (section + 1)*my_cases->section_size;
    }
    return (my_cases->array_size);
}

//returns the total number of sections
int get_total_sections(int size, int section_size)
{
    return ceil(size / (double) section_size);

}

//searching the array
unsigned char search_single_thread(int section, int value, struct test_cases * my_cases)
{
    int i = section_first_index(section, my_cases->section_size);
    int last_i = section_last_index(section, my_cases);

    for (; i < last_i; i++) {
        if (myArray[i] == value) {
            return i % my_cases->section_size;
        }
    }
    return NOT_FOUND;
}

int current_section = CUR_SECTION_INIT;
pid_t parent_pid = 0;

//returns standard deviation
double stand_dev(double run_times[], int size)
{
    double sum = 0.0;
    double mean;
    double standard_deviation = 0.0;
    int i;
    for (i = 0; i < size; ++i) {
        sum += run_times[i];
    }
    mean = sum / (double) size;
    for (i = 0; i < size; ++i)
        standard_deviation += pow(run_times[i] - mean, 2);
    return sqrt(standard_deviation / (double) size);
}


#define NUM_TESTS (sizeof(my_cases)/sizeof(struct test_cases))
//Test cases definition,
// First value -> Array Size
// Second value -> Section size
struct test_cases my_cases[] = {
   
    {500, 250},
    {1000, 250},
    {5000, 250},
    {15000, 250},
    {25000, 250},

    {25000, 50},
    {25000, 100},
    {25000, 200}
  };

// Total number of iterations per test.
#define MAX_RUNS 50

int main()
{
    int j = 0;
    for (; j < NUM_TESTS; j++) {

        printf(
                "\n################\n"
                "#### Test %d ####\n"
                "################\n", j);
        printf("Mode: %s\n", MODE);
        printf("Search Value: %d\n", SEARCH_VALUE);
        printf("Array Size: %d\nSection Size: %d \n", my_cases[j].array_size, my_cases[j].section_size);

        // Creates Array + fills it with values and shuffles.
        makeArray(my_cases[j].array_size, my_cases[j].section_size);
        int total_sections = get_total_sections(my_cases[j].array_size, my_cases[j].section_size);

        printf("Total Sections: %d\n", total_sections);
        printf("# of iterations: %d\n\n", MAX_RUNS);

        double max_t = 0.0;
        double min_t = 10000.0;
        uint64_t start_t;
        uint64_t end_t;
        double total_t = 0.0;
        double run_times[MAX_RUNS];
        int result;

        int i = 0;
        for (; i < MAX_RUNS; i++) {
            printf("--> Iteration: %d\n", i);

            start_t = mygettime();
            //Searches according to what MY_SEARCH is (search_multiprocess / search_multithread)
            result = MY_SEARCH(myArray, &my_cases[j], total_sections, SEARCH_VALUE);
            
            if(result == -1){
                exit(0);
            }
            end_t = mygettime();

            double total_run_t = (end_t - start_t) / 1000.0;
            total_t = total_t + total_run_t;
            run_times[i] = total_run_t;

            if (total_run_t > max_t) {
                max_t = total_run_t;
            }
            if (total_run_t < min_t) {
                min_t = total_run_t;
            }
            if (result == -1) {
                printf("ERROR: Value was NOT found \n");
            } else {
                printf("Value %d was found at index: %d\nTime taken: %lf milliseconds\n",
                        myArray[result],
                        result,
                        total_run_t);
            }
            if (i != MAX_RUNS - 1)
                shuffleIndex_ex(myArray, my_cases[j].array_size, result);
            //sleep(1);
        }


        printf("\n*************\nTime taken for Test %d in milliseconds\n", j);
        printf("Total: %lf \n", total_t);
        printf("Average time: %lf \n", (total_t / MAX_RUNS));
        printf("MIN time: %lf \n", min_t);
        printf("MAX time: %lf \n", max_t);
        printf("Deviation time: %lf \n", stand_dev(run_times, MAX_RUNS));
      
        // test result summary
//        printf("Total, Average time, MIN time, MAX time, Deviation time   \n");
//        printf(" %lf, %lf, %lf, %lf, %lf \n ", total_t, (total_t / MAX_RUNS), min_t, max_t, stand_dev(run_times, MAX_RUNS));

    }

    free(myArray);
    free(myResults);

    sleep(1);
    return 0;
}



