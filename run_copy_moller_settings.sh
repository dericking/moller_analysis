#!/bin/bash

STARTRUN=$1
if [[ -z ${2} ]]; then
ENDRUN=$1
else
ENDRUN=${2}
fi

ROOTDIR=${MOLLER_ROOTFILE_DIR} #THINGS GO HERE
DATADIR=${MOLLER_DATA_DIR}
SETSDIR=/adaqfs/home/moller/daq/coda2/RunInfo

for i in $(seq $STARTRUN $ENDRUN);do
    echo "Copying ${SETSDIR}/mollerrun_${i}.set to $DATADIR";
    SETFILE=${SETSDIR}/mollerrun_${i}.set
    cp $SETFILE $ROOTDIR
done
