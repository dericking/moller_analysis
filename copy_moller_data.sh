#!/bin/bash

FILEDIR=$1

STARTRUN=$2
if [[ -z ${3} ]]; then
ENDRUN=$2
else
ENDRUN=${3}
fi

DATADIR="/adaq1/data1/moller"
SETSDIR="/adaqfs/home/moller/daq/coda2/RunInfo"

for i in $(seq $STARTRUN $ENDRUN);do
    FILE2=${DATADIR}/moller_data_${i}.nt
    cp $FILE2 $FILEDIR/
    FILE3=${SETSDIR}/mollerrun_${i}.set
    cp $FILE3 $FILEDIR/
done
