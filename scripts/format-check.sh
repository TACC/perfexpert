#!/bin/sh
SCRIPT_DIR=`dirname $BASH_SOURCE`
ABS_PATH=`readlink -e $SCRIPT_DIR`

# Set the kind of files that we want to check for the coding style.
if [ $# == 0 ]
then
    type=mod
else
    type="$1"
fi

# Get the list of files.
case "$type" in
    mod)
        file_list=`git ls-files --modified`
        ;;

    staged)
        file_list=`git diff --name-only --cached`
        ;;

    *)
        # We received an invalid option.
        echo "USAGE: $0 [(mod|staged)]"
        exit 1
esac

# If we have any files to check, run cpplint.py on them.
# Otherwise, print a helpful message and quit.
if [ -n "$file_list" ]
then
    "${ABS_PATH}/cpplint.py" $file_list 2>&1 | less
else
    echo "No file(s) to check."
fi
