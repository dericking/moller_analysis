#!/bin/bash

STARTRUN=$1
if [[ -z ${2} ]]; then
ENDRUN=$1
else
ENDRUN=${2}
fi

DATADIR=${MOLLER_DATA_DIR}
SETSDIR=${MOLLER_SETTINGS_DIR}

for i in $(seq $STARTRUN $ENDRUN);do
    SETFILE=${SETSDIR}/mollerrun_${i}.set
    cp $SETFILE $FILEDIR/
done
