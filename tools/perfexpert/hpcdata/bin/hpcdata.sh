#!/usr/bin/env bash

HPCDATA_HOME=`dirname ${0}`;

LOG4J_CONFIG="-Dlog4j.configuration=file:${HPCDATA_HOME}/../../lib/log4j.properties"
CLASSPATH="${HPCDATA_HOME}/hpcdata.jar:${HPCDATA_HOME}/../../lib/argp-1.0.jar:${HPCDATA_HOME}/../../lib/log4j-1.2.16.jar"

java ${LOG4J_CONFIG} -cp ${CLASSPATH} edu.utexas.tacc.hpcdata.HPCData ${*};
