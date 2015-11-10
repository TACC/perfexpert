# PerfExpert Tutorial

This is a simple tutorial that shows how to use some of the most relevant functionality of PerfExpert 4.2.

## Compiling your application

Your executable needs to include debugging symbols (compile with -g). It is also recommended to include the desired optimization flags in your compilation proccess.

## Basic Usage



## Advanced Usage

Depending on the system that you are using and the installation of PerfExpert, you might have access to different modules within PerfExpert. For example, you might have access to VTune.

### Using VTune

If Intel VTune is installed in your system, PerfExpert can use it instead of HPCToolkit to collect the information that it needs. In order to use VTune, simply run the following command:

    $ perfexpert --modules=vtune 0.1 -- ./backprop 70000

### Calling MACPO

### Recompiling your application within PerfExpert

PerfExpert provides three modules that will automatically try to recompile your code:

* icc
* gcc
* make

We strongly recommend that you create an standarized Makefile in your project so that all you need to do to compile your application is run 'make'.

The icc and gcc modules accept an option 'source' where you can indicate the source files that are part of your application. These files will be compiled when the compilation process is required.

An example of how to use icc/gcc is as follows:

    $ perfexpert --modules=icc,lcpi --module-option=icc,source=backprop.c 0.1 -- ./backprop 70000

An example for the make module:

    $ perfexpert --modules=make,lcpi 0.1 -- ./backprop 70000

In this case, the make command will be executed in the current folder, so it expects to find a Makefile in the folder.

## Authors

Antonio Gomez-Iglesias (agomez@tacc.utexas.edu), Leonardo Fialho, Ashay Rane and James Browne

## COPYRIGHT

Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.

Additional copyrights may follow

This file is part of PerfExpert.

PerfExpert is free software: you can redistribute it and/or modify it under
the terms of the The University of Texas at Austin Research License

PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

