#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <chrono>
#include <iostream>

#define NUM_CORES 16
#define NUM_THREADS 100
#define NUM 10000
#define ITTERATIONS 10000

int MOV_DATA[NUM_THREADS][NUM];
int XOR_DATA[NUM_THREADS][NUM];

long long RESULTS[NUM_THREADS];

typedef void (*SwapFunction)(int*, int*);

struct WorkerArgs {
    SwapFunction swapFn;
    int threadID;
    int* data;
};

void swapMov(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

void swapXor(int* a, int* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void testSwapThread(SwapFunction swapFn, int thread, int* data) {
    auto t = std::chrono::high_resolution_clock::now();

    // every thread has a decent amount of work to do
    for(int i = 0; i < ITTERATIONS; i++){
        for (int j = 0; j < NUM; j += 2) {
            swapFn(&data[j], &data[j + 1]); 
        }
    }
    
    auto d = std::chrono::high_resolution_clock::now() - t;
    RESULTS[thread] = std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

void* worker(void* arg) {
    WorkerArgs* args = reinterpret_cast<WorkerArgs*>(arg);
    SwapFunction swap_fn = args->swapFn;
    int* data = args->data;
    int threadID = args->threadID;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(threadID%NUM_CORES, &cpuset); // all set to CPU 0

    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    testSwapThread(swap_fn, threadID, data);
    free(args);
    return NULL;
}

void testSwap(const SwapFunction swapFunctionPtr, const char* testName, int data[][NUM]) {
    pthread_t threads[NUM_THREADS];

    // Measure swap times
    for (int i = 0; i < NUM_THREADS; i++) {
        WorkerArgs* args = new WorkerArgs{swapFunctionPtr, i, data[i]}; // Pass the row of the 2D array
        pthread_create(&threads[i], NULL, worker, (void*)args);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calculate average time for the swap function
    long long total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total += RESULTS[i];
    }
    double average = static_cast<double>(total) / NUM_THREADS;
    std::cout << "Average time for " << testName << " swap: " << average << " microseconds" << std::endl;
}

int main() {
    testSwap(swapMov, "MOV", MOV_DATA);
    testSwap(swapXor, "XOR", XOR_DATA);
    return 0;
}
