MACPO is an acronym for Memory Access Characterization for Performance
Optimization. It is a tool that has been built to assist performance
tuning of single- and multi-threaded C, C++ or Fortran applications.
More specifically, MACPO is designed to provide insight into an
application's memory usage patterns.

A Quick Demonstration
---------------------

To demonstrate the functioning of MACPO, let's consider an example
program. The complete code is in the `tools/macpo/examples`
directory.

The following code shows the function that is executed by each thread:

    void* thread_func (void* arg) {
        int idx, i, repeat;
        float x, y, z;
        thread_info_t* thread_info = (thread_info_t*) arg;

        for (repeat = 0; repeat < REPEAT_COUNT; repeat++) {
            for (i = 0; i < ITERATIONS; i++) {
                idx = i + thread_info->tid;
                x = random_numbers[idx % RANDOM_BUFFER_SIZE];
                y = random_numbers[(1 + idx) % RANDOM_BUFFER_SIZE];
                z = x * x + y * y;
                if (z <= 1) counts[thread_info->tid]++;
            }
        }

        pthread_exit(0);
    }

To compile the application using MACPO, indicating that we are
interested in understanding the performance metrics associated with the
`thread_func` function, run the following commands:

    $ macpo.sh --macpo:instrument=thread_func -c instrument.c
    $ macpo.sh --macpo:instrument=thread_func insrument.o -o insrument -lpthread

Runtime logs (macpo.out) are produced when the application is run:

    $ ./instrument

To print performance metrics, use the `macpo-analyze` program.

    $ macpo-analyze macpo.out

This will produce an output that is similar to the one shown below:

    [macpo] Analyzing logs created from the binary /home1/01174/ashay/ts at Mon May 12 21:56:43 2014

    [macpo] Analyzing records for latency.

    [macpo] Cache conflicts:
    var: random_numbers, conflict ratio: 0%.
    var: counts, conflict ratio: 85.3846%.

The output shows that accesses to the variable `counts` suffer from cache
conflicts (also known as cache thrashing). Using this knowledge, we can pad the
`counts` array (i.e., add dummy bytes to the array) so that each thread
accesses a different cache line.

Additional analyses that can be performed using MACPO can be seen using the
command `macpo.sh --help`.

When to Use MACPO
-----------------

MACPO may be useful to you in any of the following situations:

-   Perfexpert reports that L2 or L3 data access LCPI is high.
-   Your program uses a lot of memory or it is memory-bound.
-   CPU profiling does not show any interesting results.

If all (or most) of your application's memory accesses are irregular,
you may be able to infer the optimizations applicable to your program using MACPO.
However, for such programs, MACPO metrics may not be able to assist you
directly.

Command line options accepted by MACPO
--------------------------------------

The various options accepted by macpo.sh are available using `macpo.sh --help`.

    Usage: macpo.sh [OPTIONS] [source file | object file ]
    Wrapper script to compile code and instrument a specific function or loop.

      --macpo:instrument=<location>         Instrument memory accesses in function
                                            or loop marked by <location>.
      --macpo:check-alignment=<location>    Check alignment of data structures in
                                            function or loop marked by <location>.
      --macpo:record-tripcount=<location>   Check if the trip count of the loop
                                            identified by <location> is too low.
      --macpo:record-branchpath=<location>  Check if the branches in loop identified
                                            by <location> always (or mostly) 
                                            evaluate to true or false or are
                                            unpredictable.
      --macpo:vector-strides=<location>     Check if vectorizing the loop identified
                                            by <location> will require 
                                            gather/scatter operation(s) for array
                                            references.
      --macpo:overlap-check=<location>      Check if array references in loop at
                                            <location> overlap with one another.
      --macpo:stride-check=<location>       Check if array references in loop at
                                            <location> have unit or fixed strides.
      --macpo:backup-filename=<filename>    Save source file into <filename> before
                                            instrumenting.
      --macpo:no-compile                    Instrument code but don't compile it.
      --macpo:enable-sampling               Enable sampling mode [default].
      --macpo:disable-sampling              Disable sampling mode.
      --macpo:profile-analysis              Collect basic profiling information
                                            about the requested analysis or 
                                            instrumentation.
      --help                                Give this help list.

      In addition to above options, all options accepted by GNU compilers can be
      passed to macpo.sh.
