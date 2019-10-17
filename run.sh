#!/bin/bash

################################################################################
## FETCHES SETTINGS FILE AND DATA ROOT FILE
## TAKES ONE OR TWO ARGUMENTS -- IF ONE ANALYZES SINGLE FILE, IF TWO ANALYZES RANGE
## PROBABLY NEED TO INCLUDE SOME CHECKS TO AVOID ERRORS
## >>>> SET FDIR TO LOCATION OF YOUR DATA FOLDER
## EVENTUALLY WANT TO PUSH THIS TO WEBPAGE AND DATABASE

rm summary.txt
touch summary.txt

START=$1
if [[ -z ${2} ]]; then
END=$1
else
END=${2}
fi

echo "Starting analysis for ${START} to ${END}"

#for CURR in {$START..$END}
for (( CURR=$START; CURR<=$END; CURR++ ))
do

echo "Analyzing run ${CURR}"

FDIR="prexii_sample_data"
SETF="${FDIR}/mollerrun_${CURR}.set"
DATF="${FDIR}/moller_data_${CURR}.root"

##CYCLING IF STATEMENT
if [[ -f $SETF && -f $DATF ]];then

## Get the helicity pattern, frequency and delay number from the settings file.
## We can add more here later.

## Assign numeric value to helicity pattern
PATT=$(sed -n -e "s#\(Helicity Mode ON/OFF Random/Toggle   HELPATTERNd          : \)##p" $SETF)
if [ $PATT == "Octet" ]; then
PATN=8;
fi
if [ $PATT == "Quartet" ]; then
PATN=4;
fi

##Fetch frequency and delay - clean up delay
FREQ=$(sed -n -e "s#\(Helicity frequency                   HELFREQ              : \)##p" $SETF)
DELN=$(sed -n -e "s#\(Helicity delay                       HELDELAYd            : \)##p" $SETF)
DELN=${DELN//[!0-9]/}

##Echo fetched values - can delete or comment out later.
echo "Data file: ${DATF}"
echo "H.Pattern: ${PATN}"
echo " Frequecy: ${FREQ}"
echo "Delay Num: ${DELN}"

##Run the analysis script.
root -l <<EOF
.L eric_asym.C
eric_asym("${DATF}",${PATN},${DELN},${FREQ})
EOF

mv analysis_${CURR}.pdf output/

cat "finalStats_"${CURR}".txt" >> summary.txt
rm "finalStats_"${CURR}".txt"

else

if [[ -f $DATF ]];then
	echo "File  ${DATF}  does not exist!"
fi
if [[ -f $SETF ]];then
	echo "File  ${SETF}  does not exist!"
fi

fi ##END CYCLEING IF STATEMENT

done
