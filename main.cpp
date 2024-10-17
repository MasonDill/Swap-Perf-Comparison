#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iostream>

constexpr int NUM = 100000;
const int num_tasks = 10000;
long long RESULTS[num_tasks];

typedef void (*SwapFunction)(int*, int*);

// Function declarations
void swapMov(int* a, int* b);
void swapXor(int* a, int* b);
void testSwap(SwapFunction swapFn, int thread);
void* worker(void* arg);  // Keep the argument type as void*

// Structure to hold swap function and thread ID
struct WorkerArgs {
    SwapFunction swapFn;
    int threadID;
};

// Function definitions
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

void testSwap(SwapFunction swapFn, int thread) {
    std::vector<int> v(NUM);
    long long total;

    auto t = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < NUM; j += 2) {
        swapFn(&v[j], &v[j + 1]);  // Swap adjacent elements
    }
    auto d = std::chrono::high_resolution_clock::now() - t;
    RESULTS[thread] = std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

void* worker(void* arg) {
    // Cast the argument to WorkerArgs structure
    WorkerArgs* args = reinterpret_cast<WorkerArgs*>(arg);
    SwapFunction swap_fn = args->swapFn; // Get the function pointer
    int threadID = args->threadID; // Get the thread ID

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    testSwap(swap_fn, threadID); // Pass the thread ID to testSwap
    delete args; // Free allocated memory for WorkerArgs
    return NULL;
}

int main() {
    pthread_t threads[num_tasks];

    const SwapFunction swapMovPtr = swapMov;
    const SwapFunction swapXorPtr = swapXor;

    // Measure MOV swap times
    std::cout << "MOV:" << std::endl;
    for (int i = 0; i < num_tasks; i++) {
        WorkerArgs* args = new WorkerArgs{swapMovPtr, i}; // Create args for the worker
        pthread_create(&threads[i], NULL, worker, (void*)args);
    }

    for (int i = 0; i < num_tasks; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calculate average time for MOV swap
    long long mov_total = 0;
    for (int i = 0; i < num_tasks; i++) {
        mov_total += RESULTS[i];
    }
    double mov_average = static_cast<double>(mov_total) / num_tasks;
    std::cout << "Average time for MOV swap: " << mov_average << " microseconds" << std::endl;

    // Measure XOR swap times
    std::cout << "XOR:" << std::endl;
    for (int i = 0; i < num_tasks; i++) {
        WorkerArgs* args = new WorkerArgs{swapXorPtr, i}; // Create args for the worker
        pthread_create(&threads[i], NULL, worker, (void*)args);
    }
    for (int i = 0; i < num_tasks; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calculate average time for XOR swap
    long long xor_total = 0;
    for (int i = 0; i < num_tasks; i++) {
        xor_total += RESULTS[i];
    }
    double xor_average = static_cast<double>(xor_total) / num_tasks;
    std::cout << "Average time for XOR swap: " << xor_average << " microseconds" << std::endl;

    return 0;
}
