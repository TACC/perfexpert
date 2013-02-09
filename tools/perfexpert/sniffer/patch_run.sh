#!/bin/bash

if [ ! -f ./experiment.header.tmp ]
then
	echo "The file experiment.header.tmp was not found in the current directory, did ./bin/sniffer run without errors?"
	exit 1
fi

HEADER=`cat experiment.header.tmp`

if [ "x${HEADER}" == "x" ]
then
	echo "experiment.header.tmp appears empty, did ./bin/sniffer run without errors?"
	exit 1
fi

replaceHook="___EXPERIMENT_HEADER___"
searchString=`grep ${replaceHook} perfexpert_run_exp.default`

curr_dir=`pwd`
parent_dir=${curr_dir%/*}

sed "s/${replaceHook}/${HEADER}/" perfexpert_run_exp.default > ../perfexpert_run_exp
if [ ${?} != "0" -o "x${searchString}" == "x" ]
then
	echo -e "The file ${parent_dir}/perfexpert_run_exp could not be patched. Please patch it manually by replacing the line \"${replaceHook}\" in ${curr_dir}/perfexpert_run_exp.default with the following contents and saving it as ${parent_dir}/perfexpert_run_exp:\n"
	echo -e ${HEADER} | sed 's/\\"/\"/g'
	exit 1
fi

# Add execute permission
chmod +x ../perfexpert_run_exp

exit 0
