#!/bin/bash

# Uses ${PERFEXPERT_HOME}

# First check if all required files are present

# TODO: Clean up
if [ ! -f ./perfexpert_run_exp -o ! -f ./config/machine.properties -o ! -f ./config/lcpi.properties -o ! -f ./bin/perfexpert.jar -o ! -f ./perfexpert -o ! -f ./lib/argp-1.0.jar -o ! -f ./hpcdata/bin/hpcdata.sh -o ! -f ./hpcdata/bin/hpcdata.jar ]
then
	echo "One of the following files was not found, did \`make all' run without errors?"
	echo -e "`pwd`/perfexpert_run_exp"
	echo -e "`pwd`/config/machine.properties"
	echo -e "`pwd`/config/lcpi.properties"
	echo -e "`pwd`/bin/perfexpert.jar"
	echo -e "`pwd`/perfexpert"
	echo -e "`pwd`/lib/argp-1.0.jar"
	echo -e "`pwd`/hpcdata/bin/hpcdata.jar"
	echo -e "`pwd`/hpcdata/bin/hpcdata.sh"
	exit 1
fi

trimmed=`echo ${PERFEXPERT_HOME} | sed 's/ //g' | sed 's/\///g'`
if [ "x${trimmed}" == "x" ]
then
	# Try system-wide installation first
	PERFEXPERT_HOME=/opt/apps/perfexpert
	echo "The shell variable \${PERFEXPERT_HOME} was not set or was set to an invalid value, defaulting to /opt/apps/perfexpert"
else
	echo "Using shell variable \${PERFEXPERT_HOME} set to \"${PERFEXPERT_HOME}\""
fi

# Empty out the existing contents
trimmed=`echo ${PERFEXPERT_HOME} | sed 's/ //g' | sed 's/\///g'`
if [ "x${trimmed}" != "x" ]
then
	rm -f "${PERFEXPERT_HOME}/perfexpert_run_exp"
	rm -f "${PERFEXPERT_HOME}/config/machine.properties"
	rm -f "${PERFEXPERT_HOME}/config/lcpi.properties"
	rm -f "${PERFEXPERT_HOME}/bin/perfexpert.jar"
	rm -f "${PERFEXPERT_HOME}/perfexpert"
	rm -f "${PERFEXPERT_HOME}/lib/argp-1.0.jar"
	rm -f "${PERFEXPERT_HOME}/hpcdata/bin/hpcdata.jar"
	rm -f "${PERFEXPERT_HOME}/hpcdata/bin/hpcdata.sh"
	rm -f "${PERFEXPERT_HOME}/optimizations/opt-sug-intel.pdb"

	mkdir -p "${PERFEXPERT_HOME}"
else
	echo "\${PERFEXPERT_HOME} was not set or was set to an invalid path..."
	exit 1
fi

# Copy files into the correct locations
mkdir -p "${PERFEXPERT_HOME}/config"
mkdir -p "${PERFEXPERT_HOME}/bin"
mkdir -p "${PERFEXPERT_HOME}/lib"
mkdir -p "${PERFEXPERT_HOME}/hpcdata/bin"
mkdir -p "${PERFEXPERT_HOME}/optimizations"

install --mode="u=rwx,go=rx" ./perfexpert_run_exp "${PERFEXPERT_HOME}/perfexpert_run_exp"
install --mode="u=rw,go=r"   ./config/machine.properties "${PERFEXPERT_HOME}/config/machine.properties"
install --mode="u=rw,go=r"   ./config/lcpi.properties "${PERFEXPERT_HOME}/config/lcpi.properties"
install --mode="u=rw,go=r"   ./bin/perfexpert.jar "${PERFEXPERT_HOME}/bin/perfexpert.jar"
install --mode="u=rwx,go=rx" ./perfexpert "${PERFEXPERT_HOME}/perfexpert"
install --mode="u=rw,go=r"   ./lib/log4j-1.2.16.jar "${PERFEXPERT_HOME}/lib/log4j-1.2.16.jar"
install --mode="u=rw,go=r"   ./lib/log4j.properties "${PERFEXPERT_HOME}/lib/log4j.properties"
install --mode="u=rw,go=r"   ./lib/argp-1.0.jar "${PERFEXPERT_HOME}/lib/argp-1.0.jar"
install --mode="u=rw,go=r"   ./hpcdata/bin/hpcdata.jar "${PERFEXPERT_HOME}/hpcdata/bin/hpcdata.jar"
install --mode="u=rwx,go=rx" ./hpcdata/bin/hpcdata.sh "${PERFEXPERT_HOME}/hpcdata/bin/hpcdata.sh"
install --mode="u=rw,go=r"   ./optimizations/opt-sug-intel.pdb "${PERFEXPERT_HOME}/optimizations/opt-sug-intel.pdb"

echo -e "\nPerfExpert installation is complete! You can now use PerfExpert from \"${PERFEXPERT_HOME}\".\nFor usage guide, please refer: http://www.tacc.utexas.edu/perfexpert/perfexpert-usage-guide/"
