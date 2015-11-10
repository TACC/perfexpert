# MACPO module

## Author

Antonio Gomez-Iglesias

## About this module

This module calls the MACPO tool (look in the tool/macpo folder) to instrument the code ("macpo.sh"). After compiling the code and running it, it analyzes the results of the instrumentation by using the "macpo-analyze" tool.

## Known issues

1. The compilation process once the code has been instrumented is not fully integrated
2. The instrumentation process does not work as expected when the same file is instrumented more than once. This is still work in progress.

## COPYRIGHT

Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.

Additional copyrights may follow

This file is part of PerfExpert.

PerfExpert is free software: you can redistribute it and/or modify it under
the terms of the The University of Texas at Austin Research License

PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.
