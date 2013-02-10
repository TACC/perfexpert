/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane
 */

#ifndef X86_GENERIC_H
#define X86_GENERIC_H

#ifndef __x86_64
#include "x86_specifics.h"
#else /* __x86_64 */
#include "x86-64_specifics.h"
#endif /* __x86_64 */

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define overhead_calc(init_block, count, label_prefix) \
    start_timer()                                      \
                                                       \
    /* Set up the loop */                              \
    MOV"    $"#count", %%"CX"\n\t"                     \
    XOR"    %%"BX", %%"BX"\n\t"                        \
    init_block                                         \
                                                       \
    core("", label_prefix, overhead, "$1")             \
                                                       \
    /* End measuring */                                \
    "overhead"#label_prefix"out:\n\t"                  \
    MOV"    %%"BX", -"WORD_4"(%%"BP")\n\t"             \
    stop_timer()                                       \
                                                       \
    /* Subtract the overhead */                        \
    MOV"    -"WORD_5"(%%"BP"), %%"BX"\n\t"             \
    SUB"    %%"AX", %%"BX"\n\t"                        \
    MOV"    %%"BX", %%"AX"\n\t"

#define core(instruction_block, label_prefix, overhead_prefix, incr) \
    #overhead_prefix#label_prefix"loop:\n\t"                         \
    "cmp    $0, %%"CX"\n\t"                                          \
    "je    "#overhead_prefix#label_prefix"out\n\t"                   \
    instruction_block                                                \
    DEC"    %%"CX"\n\t"                                              \
    ADD"    "incr", %%"BX"\n\t"                                      \
    "jmp    "#overhead_prefix#label_prefix"loop\n\t"

#define measure(init_block, instruction_block, count, label_prefix) \
    start_timer()                                                   \
                                                                    \
    /* Set up the loop */                                           \
    MOV"    $"#count", %%"CX"\n\t"                                  \
    XOR"    %%"BX", %%"BX"\n\t"                                     \
    init_block                                                      \
                                                                    \
    core(instruction_block, label_prefix, , "$1")                   \
                                                                    \
    /* End measuring */                                             \
    #label_prefix"out:\n\t"                                         \
    MOV"    %%"BX", -"WORD_4"(%%"BP")\n\t"                          \
    stop_timer()                                                    \
                                                                    \
    /* Store the result (with overhead) */                          \
    MOV"    %%"AX", -"WORD_5"(%%"BP")\n\t"

#define setup() /* Create space for stack var and push regs on stack */ \
    PUSH"    %%"BP"\n\t"                                                \
    MOV"    %%"SP", %%"BP"\n\t"                                         \
    SUB"    $"WORD_5", %%"SP"\n\t"                                      \
                                                                        \
    /* Push all regs that we will use in this routine, */               \
    /* which is basically all GPRs plus a few others */                 \
    PUSH"    %%"BX"\n\t"                                                \
    PUSH"    %%"SI"\n\t"                                                \
    PUSH"    %%"DI"\n\t"

#define cleanup()                      \
    /* Pop all the regs we used */     \
    POP"    %%"DI"\n\t"                \
    POP"    %%"SI"\n\t"                \
    POP"    %%"BX"\n\t"                \
                                       \
    /* Store the results */            \
    MOV"    %%"AX", %0\n\t"            \
    MOV"    -"WORD_4"(%%"BP"), %1\n\t" \
                                       \
    /* Restore frame */                \
    ADD"    $"WORD_5", %%"SP"\n\t"     \
    POP"    %%"BP"\n\t"

#define start_timer()                      \
    XOR"    %%"AX", %%"AX"\n\t"            \
    "cpuid\n\t"                            \
    "rdtsc\n\t"                            \
    MOV"    %%"AX", -"WORD_2"(%%"BP")\n\t" \
    MOV"    %%"DX", -"WORD_3"(%%"BP")\n\t"

#define stop_timer()                       \
    XOR"    %%"AX", %%"AX"\n\t"            \
    "cpuid\n\t"                            \
    "rdtsc\n\t"                            \
    SUB"    -"WORD_2"(%%"BP"), %%"AX"\n\t" \
    "sbb    -"WORD_3"(%%"BP"), %%"DX"\n\t"

#define measure_mem_with_overhead_calc(label_prefix) \
    setup()                                          \
                                                     \
    /* Set up repeat counter */                      \
    MOV"    "WORD_4"(%%"BP"), %%"DX"\n\t"            \
    MOV"    "WORD_3"(%%"BP"), %%"CX"\n\t"            \
    MOV"    "WORD_2"(%%"BP"), %%"SI"\n\t"            \
    MOV"    "WORD_1"(%%"BP"), %%"BX"\n\t"            \
    MOV"    %%"BX", -"WORD_1"(%%"BP")\n\t"           \
                                                     \
    #label_prefix"repeat:\n\t"                       \
    start_timer()                                    \
                                                     \
    /* Do some operation */                          \
    XOR"    %%"BX", %%"BX"\n\t"                      \
    MOV"    "WORD_4"(%%"BP"), %%"DX"\n\t"            \
    MOV"    "WORD_3"(%%"BP"), %%"CX"\n\t"            \
    MOV"    "WORD_2"(%%"BP"), %%"SI"\n\t"            \
                                                     \
    core(MOV"    (%%"DX"), %%"DI"\n\t"               \
         MOV"    %%"DI", %%"DX"\n\t",                \
         label_prefix,                               \
         ,                                           \
         "%%"SI"")                                   \
                                                     \
    #label_prefix"out:\n\t"                          \
    /* End measuring */                              \
    MOV"    %%"BX", -"WORD_4"(%%"BP")\n\t"           \
                                                     \
    stop_timer()                                     \
                                                     \
    /* Check if we have to repeat */                 \
    DEC"    -"WORD_1"(%%"BP")\n\t"                   \
    "jnz    "#label_prefix"repeat\n\t"               \
                                                     \
    /* Store the result (with overhead) */           \
    MOV"    %%"AX", -"WORD_5"(%%"BP")\n\t"           \
                                                     \
    /* Do everything all over again */               \
    /* without the actual step to be measured */     \
                                                     \
    start_timer()                                    \
                                                     \
    /* Do some operation */                          \
    XOR"    %%"BX", %%"BX"\n\t"                      \
    MOV"    "WORD_4"(%%"BP"), %%"DX"\n\t"            \
    MOV"    "WORD_3"(%%"BP"), %%"CX"\n\t"            \
    MOV"    "WORD_2"(%%"BP"), %%"SI"\n\t"            \
                                                     \
    core(MOV"    %%"DI", %%"DX"\n\t",                \
         label_prefix,                               \
         overhead,                                   \
         "%%"SI"")                                   \
                                                     \
    "overhead"#label_prefix"out:\n\t"                \
    /* End measuring */                              \
    MOV"    %%"BX", -"WORD_4"(%%"BP")\n\t"           \
                                                     \
    stop_timer()                                     \
                                                     \
    /* Subtract the overhead */                      \
    MOV"    -"WORD_5"(%%"BP"), %%"BX"\n\t"           \
    SUB"    %%"AX", %%"BX"\n\t"                      \
    MOV"    %%"BX", %%"AX"\n\t"                      \
                                                     \
    cleanup()

#define measure_block_with_overhead_calc(init_block, instruction_block, \
                                         count, label_prefix)           \
    setup()                                                             \
    measure(init_block, instruction_block, count, label_prefix)         \
    overhead_calc(init_block,count,label_prefix)                        \
    cleanup()

#define measure_block_without_overhead_calc(init_block, instruction_block, \
                                            count, label_prefix)           \
    setup()                                                                \
    measure(init_block, instruction_block, count, label_prefix)            \
    cleanup()

double allocateAndTest(long size, int stride, unsigned char warmup) {
    size_t ret = 0,
           diff = 0,
           count = 0,
           my_stride = stride;
    size_t *ptr = NULL;
    size_t repeatCount = (warmup == 0 ? 1 : 2);
    int i;
    long loopCount = 1024*1024;

    if (stride > size) {
        return 0;
    }

    if (NULL == (ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr) {
        if (ENOMEM == errno) {
            printf("No memory\n");
        } else if (EINVAL == errno) {
            printf("Not aligned\n");
        } else {
            printf("Unknown error in allocating %ld size with %ld alignment\n",
                   size, size);
        }
        return 0;
    }

    for (i = 0; i < size / stride - 1; i++) {
        ptr[i * stride / sizeof(size_t)] = (size_t) &(ptr[(i + 1) *
                                                (stride / sizeof(size_t))]);
    }

    ptr[(size / stride - 1) * stride / sizeof(size_t)] = (size_t) &(ptr[0]);

repeatIfNeg:
    // Simulate a function call during entry
    __asm__ volatile(PUSH" %2\n\t" // "WORD_4"(%%"BP") ptr
                     PUSH" %3\n\t" // "WORD_3"(%%"BP") size
                     PUSH" %4\n\t" // "WORD_2"(%%"BP") stride
                     PUSH" %5\n\t" // "WORD_1"(%%"BP")  repeatCount
                     measure_mem_with_overhead_calc(cache)
                     ADD" $"WORD_4", %%"SP"\n\t"
                     : "="AX"" (diff), "="CX"" (count)
                     : "m" (ptr), "m" (loopCount), "m" (my_stride), "m" (repeatCount)
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    free(ptr);
    return diff / ((float) (count / ((float) stride)));
}

double allocateAndTestMainMemory(long size, int stride, unsigned char warmup) {
    size_t ret = 0,
           diff = 0,
           count = 0,
           my_stride = stride;
    size_t *ptr = NULL;
    size_t *mem = NULL;
    size_t repeatCount = (warmup == 0 ? 1 : 2);
    long elements = size / stride-1;  // Assuming completely divisible
    long loopCount = 1024*1024;
    int i;
    int *swapBuffer = malloc(sizeof(int)*elements);
    
    
    if (stride > size) {
        return 0;
    }

    if (NULL == (ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr) {
        if (ENOMEM == errno) {
            printf("No memory\n");
        } else if (EINVAL == errno) {
            printf("Not aligned\n");
        } else {
            printf("Unknown error in allocating %ld size with %ld alignment\n",
                   size, size);
        }
        return 0;
    }

    mem = (size_t *) ptr;
    if (NULL == swapBuffer) {
        // Try the usual +512 skip
        for (i=0; i<elements-1; i++) {
            *mem = (size_t) &ptr [(i + 1) * stride / sizeof(size_t)];
            mem = (size_t*) *mem;
        }
        // Loop back
        *mem = (size_t) &ptr[0];
    } else {
        srand(time(NULL));
        int randomNumber;
        for (i = 0; i < elements; i++) {
            swapBuffer[i] = i;
        }
        // > 0 is on purpose, should not generate zero as one of the random numbers
        for (i = elements - 1; i > 0; i--) {
            int temp = i*((float) rand() / RAND_MAX);
            randomNumber = swapBuffer [temp];
            swapBuffer[temp] = swapBuffer[i];
            *mem = (size_t) &(ptr[randomNumber * stride / sizeof(size_t)]);
            mem = (size_t*) *mem;
        }
        // Loop back to the first location
        *mem = (size_t) ptr;
        free(swapBuffer);
    }

repeatIfNeg:
    // Simulate a function call during entry
    __asm__ volatile(PUSH" %2\n\t" // "WORD_4"(%%"BP") ptr
                     PUSH" %3\n\t" // "WORD_3"(%%"BP") size
                     PUSH" %4\n\t" // "WORD_2"(%%"BP") stride
                     PUSH" %5\n\t" // "WORD_1"(%%"BP") repeatCount
                     measure_mem_with_overhead_calc(MM)
                     ADD" $"WORD_4", %%"SP"\n\t"
                     : "="AX"" (diff), "="CX"" (count)
                     : "m" (ptr), "m" (loopCount), "m" (my_stride), "m" (repeatCount)
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    free(ptr);
    return diff / ((float) (count / ((float) stride)));
}

double getFPLatency() {
    size_t diff, count;
    float op1 = 1424.4525;
    float op2 = 0.5636;

repeatIfNeg:
    // Simulate a function call during entry
    __asm__ volatile(PUSH" %2\n\t" // "WORD_4"(%%"BP") ptr
                     PUSH" %3\n\t" // "WORD_2"(%%"BP") size
                     measure_block_with_overhead_calc(
                                                      "fld "WORD_2"(%%"BP")\n\t",

                                                      "fadd "WORD_2"(%%"BP")\n\t"
                                                      "fsub "WORD_1"(%%"BP")\n\t",
                                                      65536, fp)

                     ADD" $"WORD_2", %%"SP"\n\t"
                     : "="AX"" (diff), "="CX"" (count)
                     : "m" (op1), "m" (op2)
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    return diff / ((float) (count * 2));
}

double getFPSlowLatency() {
    size_t diff, count;
    float op1 = 1424.4525;
    float op2 = 0.5636;

repeatIfNeg:
    // Simulate a function call during entry
    __asm__ volatile(PUSH" %2\n\t" // "WORD_4"(%%"BP") ptr
                     PUSH" %3\n\t" // "WORD_2"(%%"BP") size
                     measure_block_with_overhead_calc(
                                                      "fld "WORD_2"(%%"BP")\n\t",
                                                      "fdiv "WORD_1"(%%"BP")\n\t"
                                                      "fsqrt\n\t",
                                                      65536, fpslow)
                     ADD"    $"WORD_2", %%"SP"\n\t"
                     : "="AX"" (diff), "="CX"" (count)
                     : "m" (op1), "m" (op2)
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    return diff / ((float) (count * 2));
}

double getPredictedBranchLatency() {
    size_t diff, count;

repeatIfNeg:
    __asm__ volatile(measure_block_with_overhead_calc(
                                                      XOR" %%"AX", %%"AX"\n\t",
                                                      CMP" $0, %%"AX"\n\t"
                                                      "je pbranchnext\n\t"
                                                      "pbranchnext:\n\t",
                                                      65536, pbranch)
                     : "="AX"" (diff), "="CX"" (count)
                     :
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    return diff / 65536.0;
}

double getMisPredictedBranchLatency() {
    size_t diff, count;

repeatIfNeg:
    __asm__ volatile(measure_block_without_overhead_calc("",
                                                         "jmp labelM01\n\t"
                                                         "labelM01: jmp labelM02\n\t"
                                                         "labelM02: jmp labelM03\n\t"
                                                         "labelM03: jmp labelM04\n\t"
                                                         "labelM04: jmp labelM05\n\t"
                                                         "labelM05: jmp labelM06\n\t"
                                                         "labelM06: jmp labelM07\n\t"
                                                         "labelM07: jmp labelM08\n\t"
                                                         "labelM08: jmp labelM09\n\t"
                                                         "labelM09: jmp labelM10\n\t"
                                                         "labelM10: jmp labelM11\n\t"
                                                         "labelM11: jmp labelM12\n\t"
                                                         "labelM12: jmp labelM13\n\t"
                                                         "labelM13: jmp labelM14\n\t"
                                                         "labelM14: jmp labelM15\n\t"
                                                         "labelM15: jmp labelM16\n\t"
                                                         "labelM16: jmp labelM17\n\t"
                                                         "labelM17: jmp labelM18\n\t"
                                                         "labelM18: jmp labelM19\n\t"
                                                         "labelM19: jmp labelM20\n\t"
                                                         "labelM20: jmp labelM21\n\t"
                                                         "labelM21: jmp labelM22\n\t"
                                                         "labelM22: jmp labelM23\n\t"
                                                         "labelM23: jmp labelM24\n\t"
                                                         "labelM24: jmp labelM25\n\t"
                                                         "labelM25: jmp labelM26\n\t"
                                                         "labelM26: jmp labelM27\n\t"
                                                         "labelM27: jmp labelM28\n\t"
                                                         "labelM28: jmp labelM29\n\t"
                                                         "labelM29: jmp labelM30\n\t"
                                                         "labelM30: jmp labelM31\n\t"
                                                         "labelM31: jmp labelM32\n\t"
                                                         "labelM32: jmp labelM33\n\t"
                                                         "labelM33: jmp labelM34\n\t"
                                                         "labelM34: jmp labelM35\n\t"
                                                         "labelM35: jmp labelM36\n\t"
                                                         "labelM36: jmp labelM37\n\t"
                                                         "labelM37: jmp labelM38\n\t"
                                                         "labelM38: jmp labelM39\n\t"
                                                         "labelM39: jmp labelM40\n\t"
                                                         "labelM40: jmp labelM41\n\t"
                                                         "labelM41: jmp labelM42\n\t"
                                                         "labelM42: jmp labelM43\n\t"
                                                         "labelM43: jmp labelM44\n\t"
                                                         "labelM44: jmp labelM45\n\t"
                                                         "labelM45: jmp labelM46\n\t"
                                                         "labelM46: jmp labelM47\n\t"
                                                         "labelM47: jmp labelM48\n\t"
                                                         "labelM48: jmp labelM49\n\t"
                                                         "labelM49: jmp labelM50\n\t"
                                                         "labelM50: jmp labelM51\n\t"
                                                         "labelM51: jmp labelM52\n\t"
                                                         "labelM52: jmp labelM53\n\t"
                                                         "labelM53: jmp labelM54\n\t"
                                                         "labelM54: jmp labelM55\n\t"
                                                         "labelM55: jmp labelM56\n\t"
                                                         "labelM56: jmp labelM57\n\t"
                                                         "labelM57: jmp labelM58\n\t"
                                                         "labelM58: jmp labelM59\n\t"
                                                         "labelM59: jmp labelM60\n\t"
                                                         "labelM60: jmp labelM61\n\t"
                                                         "labelM61: jmp labelM62\n\t"
                                                         "labelM62: jmp labelM63\n\t"
                                                         "labelM63: jmp labelM64\n\t"
                                                         "labelM64:\n\t",
                                                         1, mpbranch)
                     : "="AX"" (diff), "="CX"" (count)
                     :
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    return diff / 64.0;
}

double getTLBLatency(long size) {
    int i;
    int warmup = 0;
    int unit_inc = stride / sizeof(size_t);
    size_t ret = 0,
           diff = 0,
           count = 0;
    size_t stride = getpagesize();
    size_t repeatCount = (warmup == 0 ? 1 : 2);
    size_t *ptr;


    if (NULL == (ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr) {
        if (ENOMEM == errno) {
            printf("No memory\n");
        } else if (EINVAL == errno) {
            printf("Not aligned\n");
        } else {
            printf("Unknown error in allocating %ld size with %ld alignment\n",
                   size, size);
        }
        return 0;
    }

    for (i = 0; i < size / stride - 1; i++) {
        ptr[i * unit_inc] = (size_t) &(ptr[(i + 1) * unit_inc]);
    }
    ptr[(size / stride - 1) * unit_inc] = (size_t) &(ptr[0]);

    size = size / stride; // WTF??

repeatIfNeg:
    // Simulate a function call during entry
    __asm__ volatile(PUSH" %2\n\t" // "WORD_4"(%%"BP") ptr
                     PUSH" %3\n\t" // "WORD_3"(%%"BP") size
                     PUSH" %4\n\t" // "WORD_2"(%%"BP") stride
                     PUSH" %5\n\t" // "WORD_1"(%%"BP")  repeatCount
                     measure_mem_with_overhead_calc(tlb)
                     ADD"    $"WORD_4", %%"SP"\n\t"
                     : "="AX"" (diff), "="CX"" (count)
                     : "m" (ptr), "m" (size), "m" (stride), "m" (repeatCount)
                     : ""DX"");

    if (0 > ((long) diff)) {
        goto repeatIfNeg;
    }
    free(ptr);
    return diff / ((float) (count / ((float) stride)));
}

#endif /* X86_GENERIC_H */
