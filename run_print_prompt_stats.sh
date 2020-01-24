#!/bin/bash

# WANT TO EXPAND THIS TO PRINT EITHER THE LAST 25 
# OR JUST A SINGLE RUN
# OR A RANGE OF RUNS

PRINTLINE=0
echo " "
echo " "
echo " "
echo " "
echo " "
echo ">>> LAST 25 SUCCESSFULLY PROMPTED RUNS <<<"
for PROMPTSTATFILE in `ls -t ${MOLLER_ROOTFILE_DIR}/prompt_stats/molana_*_stats_* | head -n 25 | tac`; do
    if [ $PRINTLINE -eq "0" ]; then
        awk 'BEGIN{"wc -l < file" | getline b}{if(NR>b-2) print}' $PROMPTSTATFILE
    else
        awk 'END{print}' $PROMPTSTATFILE
    fi
    PRINTLINE=$((PRINTLINE+1))
done

