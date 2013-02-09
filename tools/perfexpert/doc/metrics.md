CSS: style.css

PerfExpert Metrics
==================

This document explains the metrics shown by PerfExpert. We will consider the following sample output:

    Input file: "perfexpert-lab/02-matmult/xml/longhorn/exp-naive.xml"
    Total running time for "perfexpert-lab/02-matmult/xml/longhorn/exp-naive.xml" is 7.559 sec
    
    Loop in function compute() (99.9% of the total runtime)
    ===============================================================================
    ratio to total instrns         %  0.........25...........50.........75........100
       - floating point        :    6 ***
       - data accesses         :   33 ****************
    
    * GFLOPS (% max)           :    7 ***
       - packed                :    0 *
       - scalar                :    7 ***
    
    -------------------------------------------------------------------------------
    performance assessment       LCPI good......okay......fair......poor......bad....
    * overall                  :  0.8 >>>>>>>>>>>>>>>>
    upper bound estimates
    * data accesses            :  2.4 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
       - L1d hits              :  1.3 >>>>>>>>>>>>>>>>>>>>>>>>>>>
       - L2d hits              :  0.3 >>>>>
       - L2d misses            :  0.8 >>>>>>>>>>>>>>>>
    * instruction accesses     :  0.3 >>>>>>
       - L1i hits              :  0.3 >>>>>>
       - L2i hits              :  0.0 >
       - L2i misses            :  0.0 >
    * data TLB                 :  1.5 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    * instruction TLB          :  0.0 >
    * branch instructions      :  0.0 >
       - correctly predicted   :  0.0 >
       - mispredicted          :  0.0 >
    * floating-point instr     :  0.2 >>>>
       - fast FP instr         :  0.2 >>>>
       - slow FP instr         :  0.0 >

Apart from the total running time, PerfExpert output includes, for each code segment:

1. Details of program composition (ratio to total instructions)
2. Approximate information about the computational efficiency (GFLOPs measurements)
3. Overall performance
4. Local Cycles Per Instruction (LCPI) values for different categories

The program composition part shows what percentage of the total instructions were computational (floating-point instructions) and what percentage were instructions that accessed data. This gives a rough estimate in trying to understand whether optimizing the program for either data accesses or floating-point instructions would have a significant impact on the total running time of the program.

The PerfExpert output also shows the GFLOPs rating, which is the number of floating-point operations executed per second in multiples of 10<sup>9</sup>. The value for this metric is displayed as a percentage of the maximum possible GFLOP value for that particular machine. Although it is rare for real-world programs to match even 50% of the maximum value, this metric can serve as an estimate of how efficiently the code performs computations.

The next, and major, section of the PerfExpert output shows the LCPI values, which is the ratio of cycles spent in the code segment for a specific category, divided by the total number of instructions in the code segment. The _overall_ value is the ratio of the total cycles taken by the code segment to the total instructions executed in the code segment. Generally, a value of 0.5 or lower for the CPI is considered to be good. The rest of the output tries to map this overall CPI, into the six constituent categories: Data accesses, Instruction accesses, Data TLB accesses, Instruction TLB accesses, Branches and Floating point computations. Without getting into the details of instruction operation on Intel and AMD chips, one can say that these six categories record performance in non-overlapping ways. That is, they roughly represent six separate categories of performance for any application. The LCPI value is a good indicator of the _penalty_ arising from instructions of the specific category. Hence higher the LCPI, the slower the program. The following is a brief description of each of these categories:

* **Data accesses**: Counts the LCPI arising from accesses to memory for program variables
* **Instruction accesses**: Counts the LCPI arising from memory accesses for code (functions and loops)
* **Data TLB**: Provides an approximate measure of penalty arising from strides in accesses or _regularity_ of accesses
* **Instruction TLB**: Reflects penalty from fetching instructions due to irregular accesses
* **Branch instructions**: Counts penalty from jumps (i.e. `if` statements, loop conditions, etc.)
* **Floating-point instructions**: Counts LCPI from executing computational (floating-point) instructions

Some of these LCPI categories can be broken down into subcategories. For instance, the LCPI from data and instruction accesses can be divided into LCPI arising from the individual levels of the data and instruction caches and branch LCPIs can be divided into LCPIs from correctly predicted and from mis-predicted branch instructions. For floating-point instructions, the division is based on floating-point instructions that take few cycles to execute (e.g. add, subtract and multiply instructions) and on floating-point instructions that take longer to execute (e.g. divide and square-root instructions).

* * *
