#!/bin/bash

################################################################################
## FETCHES SETTINGS FILE AND DATA ROOT FILE
## TAKES ONE OR TWO ARGUMENTS -- IF ONE ANALYZES SINGLE FILE, IF TWO ANALYZES RANGE
## PROBABLY NEED TO INCLUDE SOME CHECKS TO AVOID ERRORS
## >>>> SET FDIR TO LOCATION OF YOUR DATA FOLDER
## EVENTUALLY WANT TO PUSH THIS TO WEBPAGE AND DATABASE

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

if [[ -f $SETF && -f $DATF ]];then

## Get the helicity pattern, frequency and delay number from the settings file.
## We can add more here later.

PATT=$(sed -n -e "s#\(Helicity Mode ON/OFF Random/Toggle   HELPATTERNd          : \)##p" $SETF)
if [ $PATT == "Octet" ]; then
PATN=8;
fi
if [ $PATT == "Quartet" ]; then
PATN=4;
fi

FREQ=$(sed -n -e "s#\(Helicity frequency                   HELFREQ              : \)##p" $SETF)

DELN=$( sed -n -e "s#\(Helicity delay                       HELDELAYd            : \)##p" $SETF)
DELN=${DELN//[!0-9]/}

echo "Data file: ${DATF}"
echo "H.Pattern: ${PATN}"
echo " Frequecy: ${FREQ}"
echo "Delay Num: ${DELN}"

root -l <<EOF
.L eric_asym.C
eric_asym("${DATF}",${PATN},${DELN},${FREQ})
EOF

mv analysis_${CURR}.pdf output/

else

if [[ -f $DATF ]];then
	ifecho "File  ${DATF}  does not exist!"
fi
if [[ -f $SETF ]];then
	echo "File  ${SETF}  does not exist!"
fi


fi

done
