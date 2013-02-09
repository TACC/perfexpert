CSS: style.css

PerfExpert Installation Instructions
====================================

Prerequisites
-------------

PerfExpert is based on other tools and thus requires that they be installed. These are:

* **PAPI** (<http://icl.cs.utk.edu/papi/software/index.html>): PAPI is required to measure hardware performance metrics like cache misses, branch instructions, etc. The PAPI installation is mostly straight-forward: Download, configure, make and make install. If your Linux kernel version is 2.6.32 or higher, then PAPI will mostly likely use `perf_events`. Recent versions of PAPI (v3.7.2 and beyond) support using `perf_events` in the Linux kernel. However, if your kernel version is lower than 2.6.32, then you would require patching the kernel with either `perfctr` (<http://user.it.uu.se/~mikpe/linux/perfctr/2.6/>) or `perfmon` (<http://perfmon2.sourceforge.net/>).

* **HPCToolkit** (<http://hpctoolkit.org/software.html>): HPCToolkit is a tool that works on top of PAPI. HPCToolkit is used by PerfExpert to run the program multiple times with specific performance counters enabled. It is also useful for correlating addresses in the compiled binary back to the source code. HPCToolkit is comprised of three parts:

  1.  `hpctoolkit` (required by PerfExpert): Which contains the programs to run the user application with performance counters and to generate the profiles
  2.  `hpctoolkit-externals`: External libraries (e.g. `libdwarf`, `libxml2`, etc.) that are used by `hpctoolkit`

* **Java Development Kit** (<http://www.oracle.com/technetwork/java/javase/downloads/index.html>) and **Apache Ant** (<http://ant.apache.org/bindownload.cgi>): To compile PerfExpert. If you are not using Sun/Oracle JDK, you might have to set `ANT_HOME` and `JAVA_HOME`, respectively to the path where Ant and Java are installed.


Downloading PerfExpert
----------------------

The PerfExpert source code can be downloaded from: <http://www.tacc.utexas.edu/perfexpert/registration>.


Setting up PerfExpert
---------------------

PerfExpert now comes with a Makefile to automate the entire installation process. The installation process requires a few user inputs which are supplied via environment variables:

* `PAPI_HEADERS`: Absolute path to `papi.h` (optional if PAPI is installed in the standard location - `/usr`)
* `PAPI_LIBS`: Absolute path to `libpapi.a` or `libpapi.so` (optional, see `PAPI_HEADERS`)
* `PERFEXPERT_HOME`: Absolute path to location where PerfExpert should be installed (default: `/opt/apps/perfexpert`)

Once the required environment variables are set, run `make` followed by `make install`. As a final step of installing PerfExpert, the `PERFEXPERT_HPCTOOLKIT_HOME` environment variable needs to be set to the absolute path where HPCToolkit was installed

The following is a sample installation on my laptop.

    klaus@iHitch:~/.workspace/pe-github$ PAPI_HEADERS=/home/klaus/apps/papi/include/ PAPI_LIBS=/home/klaus/apps/papi/lib/ make
    `which ant` -f ./argp/build.xml clean jar
    Buildfile: /home/klaus/.workspace/pe-github/argp/build.xml
    
    clean:
    
    clean:
    
    init:
        [mkdir] Created dir: /home/klaus/.workspace/pe-github/argp/bin
    
    build-project:
         [echo] ArgP: /home/klaus/.workspace/pe-github/argp/build.xml
        [javac] /home/klaus/.workspace/pe-github/argp/build.xml:34: warning: "includeantruntime" was not set, defaulting to build.sysclasspath=last; set to false for repeatable builds
        [javac] Compiling 9 source files to /home/klaus/.workspace/pe-github/argp/bin
    
    jar:
          [jar] Building jar: /home/klaus/.workspace/pe-github/lib/argp-1.0.jar
       [delete] Deleting directory /home/klaus/.workspace/pe-github/argp/bin
    
    BUILD SUCCESSFUL
    Total time: 2 seconds
    `which ant` -f ./hpcdata/build.xml clean jar
    Buildfile: /home/klaus/.workspace/pe-github/hpcdata/build.xml
    
    clean:
    
    clean:
    
    init:
    
    build-project:
         [echo] HPCData: /home/klaus/.workspace/pe-github/hpcdata/build.xml
        [javac] /home/klaus/.workspace/pe-github/hpcdata/build.xml:35: warning: "includeantruntime" was not set, defaulting to build.sysclasspath=last; set to false for repeatable builds
        [javac] Compiling 6 source files to /home/klaus/.workspace/pe-github/hpcdata/bin
    
    jar:
          [jar] Building jar: /home/klaus/.workspace/pe-github/hpcdata/bin/hpcdata.jar
       [delete] Deleting directory /home/klaus/.workspace/pe-github/hpcdata/bin/edu
    
    BUILD SUCCESSFUL
    Total time: 2 seconds
    `which ant` -f ./build/build.xml clean jar
    Buildfile: /home/klaus/.workspace/pe-github/build/build.xml
    
    clean:
       [delete] Deleting directory /home/klaus/.workspace/pe-github/bin
    
    clean:
    
    init:
    [mkdir] Created dir: /home/klaus/.workspace/pe-github/bin
     [copy] Copying 4 files to /home/klaus/.workspace/pe-github/bin
    
    build-project:
         [echo] PerfExpert: /home/klaus/.workspace/pe-github/build/build.xml
        [javac] /home/klaus/.workspace/pe-github/build/build.xml:34: warning: "includeantruntime" was not set, defaulting to build.sysclasspath=last; set to false for repeatable builds
        [javac] Compiling 23 source files to /home/klaus/.workspace/pe-github/bin
    
    jar:
          [jar] Building jar: /home/klaus/.workspace/pe-github/bin/perfexpert.jar
    
    BUILD SUCCESSFUL
    Total time: 2 seconds
    cd hound/ && make && cd ..
    make[1]: Entering directory `/home/klaus/.workspace/pe-github/hound"
    rm -f bin/hound src/driver.o
    rm -rf bin/
    cc -fPIC    -c -o src/driver.o src/driver.c
    mkdir bin/
    cc -fPIC  src/driver.o -o bin/hound
    make[1]: Leaving directory `/home/klaus/.workspace/pe-github/hound"
    mkdir config/
    cd hound/ && ./bin/hound | tee machine.properties && mv machine.properties ../config/machine.properties && cd ..
    version = 1.0
    
    # Generated using hound
    CPI_threshold = 0.5
    L1_dlat = 2.60
    L1_ilat = 2.60
    L2_lat = 23.10
    mem_lat = 257.21
    CPU_freq = 800000000
    FP_lat = 1.89
    FP_slow_lat = 21.97
    BR_lat = 1.00
    BR_miss_lat = 6.47
    TLB_lat = 18.35
    cd sniffer/ && make CFLAGS="-I /home/klaus/apps/papi/include/" LDFLAGS="-lpapi  -L /home/klaus/apps/papi/lib/" && cd ..
    make[1]: Entering directory `/home/klaus/.workspace/pe-github/sniffer"
    rm -f bin/sniffer src/driver.o
    rm -f lcpi.properties experiment.header.tmp
    rm -rf bin/
    cc -I /home/klaus/apps/papi/include/   -c -o src/driver.o src/driver.c
    mkdir bin/
    cc -lpapi  -L /home/klaus/apps/papi/lib/ src/driver.o -o bin/sniffer
    make[1]: Leaving directory `/home/klaus/.workspace/pe-github/sniffer"
    mkdir config/
    mkdir: cannot create directory `config/": File exists
    make: [sniffer] Error 1 (ignored)
    cd sniffer/ && LD_LIBRARY_PATH=:/home/klaus/apps/papi/lib/ ./bin/sniffer && mv lcpi.properties ../config/lcpi.properties && ./patch_run.sh && cd ..
    Generated LCPI and experiment files
    
    klaus@iHitch:~/.workspace/pe-github$ PERFEXPERT_HOME=/home/klaus/apps/perfexpert make install
    ./install_perfexpert.sh
    Using shell variable ${PERFEXPERT_HOME} set to "/home/klaus/apps/perfexpert"
    
    PerfExpert installation is complete! You can now use PerfExpert from
    "/home/klaus/apps/perfexpert".
    For usage guide, please refer:
    http://www.tacc.utexas.edu/perfexpert/perfexpert-usage-guide
    
    klaus@iHitch:~/.workspace/pe-github$ export PERFEXPERT_HPCTOOLKIT_HOME=${HOME}/apps/hpctoolkit >> ~/.bashrc

* * *
