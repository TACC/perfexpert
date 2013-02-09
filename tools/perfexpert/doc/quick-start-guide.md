CSS: style.css

PerfExpert Quick Start Guide
=================

This guide explains how to run programs using PerfExpert and how to interpret its output using a sample matrix multiplication program. For this guide, we will use the sample matrix multiplication program located at: <https://webspace.utexas.edu/asr596/www/perfexpert/downloads/source.c>. This program multiplies two matrices and prints one value from the resulting matrix.

## Step 1: Measure performance

If you are using any of the [TACC](http://www.tacc.utexas.edu/) machines, load the appropriate modules:

    module load papi java perfexpert

To measure the performance of an application, run the code with `perfexpert_run_exp`. PerfExpert will run your application multiple times with different performance counters enabled.

**IMP:** When running your application with a jobscript, be sure to specify a running time that is about 6x the normal running time of the program.

    $ cat jobscript
    #!/bin/bash
    #$ -V                           # Inherit the submission environment
    #$ -cwd                         # Start job in submission directory
    #$ -N PerfExpert                # Job Name
    #$ -j y                         # Combine stderr and stdout
    #$ -o $JOB_NAME.o$JOB_ID        # Name of the output file (eg. myMPI.oJobID)
    #$ -pe 1way 8                   # Request 1 task
    #$ -q development
    #$ -l h_rt=00:03:00             # Run time (hh:mm:ss) - 3 minutes
    #$ -P hpc
     
    perfexpert_run_exp ./source
     
    $ qsub jobscript
    

This will create an experiment.xml file that contains the profiling information.

## Step 2: Determine bottlenecks

For identifying bottlenecks, run `perfexpert` on the file that was generated after the measurement phase. To get an idea of the performance characteristics of the whole program, use the `--aggregate` or `-a` option to `perfexpert`. This will display the PerfExpert output for the program that was profiled. The `0.1` in the command is a threshold (explained later). The bars (LCPI values) indicate which categories are most beneficial to optimize (long bars) and which categories can safely be ignored (short bars).

For function- and loop-level information, drop the `--aggregate` or the `-a` option from the previous command. The assessed code sections are output in order of total runtime. The threshold of `0.1` indicates that we are only interested in functions and loops that take 10% or more of the total running time.

    $ perfexpert 0.1 experiment-naive.xml
    Total running time is 7.94 sec
     
    Loop in function compute() (99.9% of the total runtime)
    ===============================================================================
    ratio to total instrns      %  0.........25...........50.........75........100
       - floating-point     :    6 ***
       - data accesses      :   33 ****************
    
    * GFLOPS (% max)        :    7 ***
    -------------------------------------------------------------------------------
    performance assessment    LCPI good......okay......fair......poor......bad....
    * overall               :  0.8 >>>>>>>>>>>>>>>>>
    upper bound estimates
    * data accesses         :  2.4 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
       - L1d hits           :  1.3 >>>>>>>>>>>>>>>>>>>>>>>>>>>
       - L2d hits           :  0.2 >>>>>
       - L2d misses         :  0.8 >>>>>>>>>>>>>>>>
    * instruction accesses  :  0.3 >>>>>>
       - L1i hits           :  0.3 >>>>>>
       - L2i hits           :  0.0 >
       - L2i misses         :  0.0 >
    * data TLB              :  1.5 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    * instruction TLB       :  0.0 >
    * branch instructions   :  0.0 >
       - correctly predicted:  0.0 >
       - mispredicted       :  0.0 >
    * floating-point instr  :  0.2 >>>>
       - fast FP instr      :  0.2 >>>>
       - slow FP instr      :  0.0 >
    

## Step 3: Optimize the code

From the PerfExpert output, we understand that the loop in function `compute()` was the most important section of the code.

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            for (k = 0; k < n; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    

The performance of this code section is bad. PerfExpert's analysis reveals data memory accesses and data TLB accesses to be the main culprits. AutoSCOPE provides suggestions on code changes and compiler flags that can help make the code section faster.

    $ perfexpert 0.1 experiment-naive.xml --recommend
    
    ********************************************************************************
    Loop in function compute() at source.c:15 (100% of the total runtime)
    ********************************************************************************
    
    change the order of loops
    This optimization may improve the memory access pattern and make it more cache
    and TLB friendly.
    
    loop i {
      loop j {...}
    }
     =====>
    loop j {
      loop i {...}
    }

For this exercise, we can quickly change the order in which the loops are executed to the following:

    for (i = 0; i < n; i++) {
        for (k = 0; k < n; k++) {
            for (j = 0; j < n; j++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    

When we measure the performance of this code, we get the following output:

    $ perfexpert 0.1 experiment-opt.xml
    Total running time is 5.053 sec
     
    Loop in function compute() (99.9% of the total runtime)
    ===============================================================================
    ratio to total instrns      %  0.........25...........50.........75........100
       - floating-point     :    6 ***
       - data accesses      :   31 **************
    
    * GFLOPS (% max)        :   10 *****
    -------------------------------------------------------------------------------
    performance assessment    LCPI good......okay......fair......poor......bad....
    * overall               :  0.5 >>>>>>>>>>>
    upper bound estimates
    * data accesses         :  2.1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
       - L1d hits           :  1.2 >>>>>>>>>>>>>>>>>>>>>>>>
       - L2d hits           :  0.0 >
       - L2d misses         :  0.8 >>>>>>>>>>>>>>>>
    * instruction accesses  :  0.3 >>>>>>
       - L1i hits           :  0.3 >>>>>>
       - L2i hits           :  0.0 >
       - L2i misses         :  0.0 >
    * data TLB              :  0.0 >
    * instruction TLB       :  0.0 >
    * branch instructions   :  0.0 >
       - correctly predicted:  0.0 >
       - mispredicted       :  0.0 >
    * floating-point instr  :  0.2 >>>>
       - fast FP instr      :  0.2 >>>>
       - slow FP instr      :  0.0 >
    

The performance is much better now as the total runtime dropped. Moreover, the data TLB is no longer a problem. Thus, PerfExpert correctly diagnosed this performance problem, suggested a useful code transformation to alleviate it, and helped verify the resolution of the problem. Since the overall performance is still bad, this optimization procedure should be repeated. Loop blocking is likely to give further improvement.

It is also possible to compare the measurements of two programs. For this, pass both experiment.xml files as arguments to `perfexpert`. For example:

    $ perfexpert 0.1  experiment-naive.xml experiment-opt.xml
     
    Loop in function compute() (runtimes are 7.935s and 5.047s)
    ===============================================================================
    ratio to total instrns      %  0.........25...........50.........75........100
       - floating-point     :      ***
       - data accesses      :      **************1
    
    * GFLOPS (% max)        :      ***2
    -------------------------------------------------------------------------------
    performance assessment    LCPI good......okay......fair......poor......bad....
    * overall               :      >>>>>>>>>>>11111
    upper bound estimates
    * data accesses         :      >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>1111+
       - L1d hits           :      >>>>>>>>>>>>>>>>>>>>>>>>11
       - L2d hits           :      >111
       - L2d misses         :      >>>>>>>>>>>>>>>>
    * instruction accesses  :      >>>>>>
       - L1i hits           :      >>>>>>
       - L2i hits           :      >
       - L2i misses         :      >
    * data TLB              :      >11111111111111111111111111111
    * instruction TLB       :      >
    * branch instructions   :      >
       - correctly predicted:      >
       - mispredicted       :      >
    * floating-point instr  :      >>>>
       - fast FP instr      :      >>>>
       - slow FP instr      :      >

The first argument passed to `perfexpert` is the XML file for the naive code whereas the second argument is the XML file for the optimized code. PerfExpert shows that the naive code performs poorly for data accesses and for data TLB in comparison with the optimized code. The overall performance, too, is slightly lower for the naive code.

## Links
PerfExpert: <http://www.tacc.utexas.edu/perfexpert><br/>
PerfExpert download: <http://www.tacc.utexas.edu/perfexpert/registration><br/>
Training Material: <http://users.ices.utexas.edu/~ashay/perfexpert/ppopp-12/>

* * *
