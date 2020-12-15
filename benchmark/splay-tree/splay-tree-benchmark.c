//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // malloc
#include <stdio.h>      // printf
#include <inttypes.h>   // PRIu64



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/splay-tree/splay-uint64.h"
#include "test-splay-tree.h"
#include "benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// private operations
//******************************************************************************

static void initialize_splay_tree() {
    // initializing the splay tree with 1k elements
    for (int i = 0; i < LOWER; i++) {
        uint64_t key = i;
        uint64_t val = key >> 1;
        test_splay_insert(key, val);
    }
}


static uint64_t random_key() {
    return (rand() % 
           (UPPER - LOWER)) + LOWER;
}


static void *benchmark1_operations(void *arg) {
    // 50% inserts, 0% finds, 50% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 2;
    
    uint64_t key, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        val = key >> 1;

        test_splay_insert(key, val);
        test_splay_delete(key);
    }
}


static void *benchmark2_operations(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 3;
    
    uint64_t key, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        val = key >> 1;

        test_splay_insert(key, val);
        result_val = test_splay_entry_val_get(key);
        test_splay_delete(key);
    }
}


static void *benchmark3_operations(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 4;
    
    uint64_t key1, key2, val, result_val1, result_val2;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        val = key1 >> 1;
        

        test_splay_insert(key1, val);
        result_val1 = test_splay_entry_val_get(key1);
        result_val2 = test_splay_entry_val_get(key2);
        test_splay_delete(key1);
    }
}


static void *benchmark4_operations(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 7;
    
    uint64_t key1, key2, key3, key4, key5, val, result_val1, result_val2, result_val3, result_val4, result_val5;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        key3 = random_key();
        key4 = random_key();
        key5 = random_key();
        val = key1 >> 1;

        test_splay_insert(key1, val);
        result_val1 = test_splay_entry_val_get(key1);
        result_val2 = test_splay_entry_val_get(key2);
        result_val3 = test_splay_entry_val_get(key3);
        result_val4 = test_splay_entry_val_get(key4);
        result_val5 = test_splay_entry_val_get(key5);
        test_splay_delete(key1);
    }
}


static void *benchmark5_operations(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 20;
    
    uint64_t key, val, result_val;

    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 17; j++) {
            key = random_key();
            result_val = test_splay_entry_val_get(key);
        }
        key = random_key();
        val = key >> 1;
        test_splay_insert(key, val);
        result_val = test_splay_entry_val_get(key);
        test_splay_delete(key);
    }
}


static void benchmark(int benchmark_no, int thread_count) {
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

    void *(*benchmark_fn_ptr)(void*);
    switch (benchmark_no) {
    case 1:
        benchmark_fn_ptr = benchmark1_operations;
        break;
    case 2:
        benchmark_fn_ptr = benchmark2_operations;
        break;
    case 3:
        benchmark_fn_ptr = benchmark3_operations;
        break;
    case 4:
        benchmark_fn_ptr = benchmark4_operations;
        break;
    case 5:
        benchmark_fn_ptr = benchmark5_operations;
        break;
    default:
        printf("benchmark not found\n");
        break;
    }

	// clock start
    struct timespec start = timer_start();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark_fn_ptr, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

    // print time
    double benchmark_time = timer_elapsed(start);
    if (!DEBUG) {
        if (thread_count != LAST_THREAD) {
            printf("%f, ", benchmark_time);
        } else {
            printf("%f", benchmark_time);
        }
    } else {
        debug_print("threads: %d, time: %f\n", thread_count, benchmark_time);
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void splay_tree_benchmark () {
    // seeding so that different random sequences get generated
    srand (time(NULL));

    debug_print("Benchmark 1: 50%% inserts, 0%% finds, 50%% deletes\n====================================================\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(1, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 2: 33%% inserts, 33%% finds, 33%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(2, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 3: 25%% inserts, 50%% finds, 25%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(3, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 4: 15%% inserts, 70%% finds, 15%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(4, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 5: 5%% inserts, 90%% finds, 5%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(5, t);
        clear_test_splay_tree();
    }
}
