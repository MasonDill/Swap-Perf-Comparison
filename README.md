# Introduction
The purpose of this program is to compare the multithreaded performance of register swapping with MOV instructions or XOR instructions.

# C Code Snippits
MOV Paradigm
```
void swapMov(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}
```
XOR Paradigm
```
void swapXor(int* a, int* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
```
# Assembly Snippits
### x86-64 gcc 13.2 with -O3 optimizations
```
swapMov(int*, int*):
        mov     eax, DWORD PTR [rdi]
        mov     edx, DWORD PTR [rsi]
        mov     DWORD PTR [rdi], edx
        mov     DWORD PTR [rsi], eax
        ret
swapXor(int*, int*):
        mov     eax, DWORD PTR [rdi]
        xor     eax, DWORD PTR [rsi]
        mov     DWORD PTR [rdi], eax
        xor     eax, DWORD PTR [rsi]
        mov     DWORD PTR [rsi], eax
        xor     DWORD PTR [rdi], eax
        ret
```
### x86-64 gcc 13.2 with -O0 optimizations
```
swapMov(int*, int*):
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-24], rdi
        mov     QWORD PTR [rbp-32], rsi
        mov     rax, QWORD PTR [rbp-24]
        mov     eax, DWORD PTR [rax]
        mov     DWORD PTR [rbp-4], eax
        mov     rax, QWORD PTR [rbp-32]
        mov     edx, DWORD PTR [rax]
        mov     rax, QWORD PTR [rbp-24]
        mov     DWORD PTR [rax], edx
        mov     rax, QWORD PTR [rbp-32]
        mov     edx, DWORD PTR [rbp-4]
        mov     DWORD PTR [rax], edx
        nop
        pop     rbp
        ret
swapXor(int*, int*):
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     QWORD PTR [rbp-16], rsi
        mov     rax, QWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rax]
        mov     rax, QWORD PTR [rbp-16]
        mov     eax, DWORD PTR [rax]
        xor     edx, eax
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax], edx
        mov     rax, QWORD PTR [rbp-16]
        mov     edx, DWORD PTR [rax]
        mov     rax, QWORD PTR [rbp-8]
        mov     eax, DWORD PTR [rax]
        xor     edx, eax
        mov     rax, QWORD PTR [rbp-16]
        mov     DWORD PTR [rax], edx
        mov     rax, QWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rax]
        mov     rax, QWORD PTR [rbp-16]
        mov     eax, DWORD PTR [rax]
        xor     edx, eax
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax], edx
        nop
        pop     rbp
        ret
```

# Test Enviroment
- Intel i9-9900K @ 3.6 GHz per core
  - Same system used for all presented test results

# Test case 1
- 1,000 Threads spawned on 1 core
- 50,000 swap operations per thread
- 100,000 unique memory addressess for each thread

### Result x86-64 gcc 13.2 with -O3 optimizations
- Average time for MOV swap: 12097 microseconds
- Average time for XOR swap: 12849 microseconds
- Differences appear minimal, with MOV having a slight edge

### Result x86-64 gcc 13.2 with -O0 optimizations
- Average time for MOV swap: 17142.1 microseconds
- Average time for XOR swap: 21242.3 microseconds
- Differences appear more substantial with MOV comfortably faster

# Test case 2
- 1,000 Threads spawned on 1 core
- 50,000 swap operations per itteration
- 100 itterations per thread
- 100,000 unique memory addressess for each thread

### Result x86-64 gcc 13.2 with -O3 optimizations
- Average time for MOV swap: 129273 microseconds
- Average time for XOR swap: 135643 microseconds
- Differences appear minimal, with MOV having a slight edge

### Result x86-64 gcc 13.2 with -O0 optimizations
- Average time for MOV swap: 3.141e+06 microseconds
- Average time for XOR swap: 7.82092e+06 microseconds
- MOV is substantially faster

# Disclaimer
The presented results representative of averages achieved on my system. Results are unique to each system (due to architecture, microarchitecture, cache size, and runtime context).
