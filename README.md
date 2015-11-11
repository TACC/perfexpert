# PerfExpert

## Authors

Antonio Gomez-Iglesias, Leonardo Fialho, Ashay Rane and James Browne

## About PerfExpert

PerfExpert combines a simple user interface with a sophisticated analysis engine to:
- Detect and diagnosis the causes for any core-, socket-, and node-level performance bottlenecks in each procedure and loop of an application.
- Apply pattern-based software transformations on the application source code to enhance performance on identified bottlenecks.- 
- Provide performance analysis report and suggestions for bottleneck remediation for application’s performance bottlenecks which we are unable to optimize automatically.


**PerfExpert is an open-source project. Funding to keep researchers working on
PerfExpert depends on the value of this tool to the scientific community. For
that reason, it is really important to know where and who are using our tool.
We would really appreciate it if you could send us a message
(agomez@tacc.utexas.edu) telling us the institution (name and country) you
are planning to install and test PerfExpert at.**

## Documentation:
--------------
See doc/ directory.

## Version History

### Version 4.2 (still in development):

- integration of VTune;
- improved support for MACPO;
- integration of hotspots and events tables in the database so that all the tools use the same tables

### Version 4.1.1:

- LLC (last level cache, usually L3) support added;
- tools were rename to perfexpert_something;
- perfexpert_analyzer now is C only, thus Apache Ant is not a requirement anymore;
- improvements and bug fixes on arguments handling:
    - prefix, before, and after (-p, -b, -a) arguments are split by space to multiple arguments;
    - quoted target program arguments are split by space to multiple arguments (bug fix);
    - it is possible to pass a quoted argument to the target program ("this \"is valid\" too");
- several improvements in MACPO;
- license updated (UT license);
- "compatibility mode" which accepts the old perfexpert_run_exp syntax (only run the experiments);
- added the option -n which does not do anything but permits users to check arguments;
- the compilation of every single tool is options using --disable-tool;
- option (-o) to sort performance bottlenecks (three sorting options available);
- the target program now supports relative path, full path, and path search;
- improved memory usage (freeing memory when possible);
- moved pre-requisites (externals) to a separated branch;
- general improvements on database installing and updating;
- improved interface with the user:
	- analyzer shows the relevance of each module;
	- code transformer shows which optimization has been applied;
	- analyzer and recommendation reports are now shown for every optimization step;
	- adding $PATH search to target programs;
	- clearer messages to the user (outside debug mode);
- other minor improvements and bug fixes.

## Known issues:

1) The sniffer tool SEGFAULT with PAPI 5.3. Please, find a patch to this issue on the contrib directory.

## COPYRIGHT

Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.

Additional copyrights may follow

This file is part of PerfExpert.

PerfExpert is free software: you can redistribute it and/or modify it under
the terms of the The University of Texas at Austin Research License

PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.
