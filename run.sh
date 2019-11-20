#!/bin/bash

################################################################################
## FETCHES SETTINGS FILE AND DATA ROOT FILE
## TAKES ONE OR TWO ARGUMENTS -- IF ONE ANALYZES SINGLE FILE, IF TWO ANALYZES RANGE
## PROBABLY NEED TO INCLUDE SOME CHECKS TO AVOID ERRORS
## >>>> SET FILEDIR TO LOCATION OF YOUR DATA FOLDER
## EVENTUALLY WANT TO PUSH THIS TO WEBPAGE AND DATABASE

#These will need to be changed.
#Where to place the analysis results? Where to find the analysis data?
RSLTDIR="/adaqfs/home/moller/nikolaev/git_asym/moller_analysis/analysis"
[ -d "${RSLTDIR}" ] && echo "Moller online analysis directory exists. Woohoo!!!" || mkdir ${RSLTDIR}
FILEDIR="/adaqfs/home/moller/nikolaev/git_asym/moller_analysis"

#remove any png from THIS directory.
rm *.png

rm summary.txt
touch summary.txt

START=$1
if [[ -z ${2} ]]; then
END=$1
else
END=${2}
fi

echo "Starting analysis for ${START} to ${END}"

#for ANALRUN in {$START..$END}
for (( ANALRUN=$START; ANALRUN<=$END; ANALRUN++ ))
do

echo "Analyzing run ${ANALRUN}"

SETFILE="${FILEDIR}/mollerrun_${ANALRUN}.set"
DATFILE="${FILEDIR}/moller_data_${ANALRUN}.root"

##CYCLING IF STATEMENT
if [[ -f $SETFILE && -f $DATFILE ]];then

## Get the helicity pattern, FREQNCYuency and delay number from the settings file.
## We can add more here later.

## Assign numeric value to helicity pattern
PATTERN=$(sed -n -e "s#\(Helicity Mode ON/OFF Random/Toggle   HELPATTERNd          : \)##p" $SETFILE)
if [ $PATTERN == "Octet" ]; then
PATTERN=8;
fi
if [ $PATTERN == "Quartet" ]; then
PATTERN=4;
fi

##Fetch FREQNCYuency and delay - clean up delay
FREQNCY=$(sed -n -e "s#\(Helicity frequency                   HELFREQ              : \)##p" $SETFILE)
HELDLAY=$(sed -n -e "s#\(Helicity delay                       HELDELAYd            : \)##p" $SETFILE)
HELDLAY=${HELDLAY//[!0-9]/}
RUNDATE=$(sed -n -e "s#\(Date       : \)##p" $SETFILE)
read dayname month datnum clock zone year <<< ${RUNDATE}
#RUNDATE=$(date --date="$(printf "${RUNDATE}")" +%Y-%m-%d_%H-%M-%S)
RUNDATE=$(date --date="$(printf "${RUNDATE}")" +%Y-%m-%d)
RUNTIME=$(date --date="$(printf "${RUNDATE}")" +%H-%M-%S)

##Echo fetched values - can delete or comment out later.
echo "Data file: ${DATFILE}"
echo "H.Pattern: ${PATTERN}"
echo "Frequency: ${FREQNCY}"
echo "Delay Num: ${HELDLAY}"
echo "Date/Time: ${RUNDATE}"

##Run the analysis script.
root -l <<EOF
.L eric_asym.C
eric_asym("${DATFILE}",${PATTERN},${HELDLAY},${FREQNCY})
EOF

[ ! -d "${RSLTDIR}/files" ]            && mkdir ${RSLTDIR}/files
[ ! -d "${RSLTDIR}/files/${ANALRUN}" ] && mkdir ${RSLTDIR}/files/run_${ANALRUN}

mv  analysis_${ANALRUN}.pdf    ${RSLTDIR}/files/run_${ANALRUN}/
cp  final_stats_${ANALRUN}.txt  ${RSLTDIR}/files/run_${ANALRUN}/
mv  errors_${ANALRUN}.txt       ${RSLTDIR}/files/run_${ANALRUN}/
mv  *.png                      ${RSLTDIR}/files/run_${ANALRUN}/

[ ! -d "${RSLTDIR}/date" ]                       && mkdir ${RSLTDIR}/date
[ ! -d "${RSLTDIR}/date/${RUNDATE}" ]            && mkdir ${RSLTDIR}/date/${RUNDATE}

ln    -s  ${RSLTDIR}/files/run_${ANALRUN}  ${RSLTDIR}/date/${RUNDATE}/

cat "final_stats_${ANALRUN}.txt" >> summary.txt
rm "final_stats_${ANALRUN}.txt"

else

if [[ -f $DATFILE ]];then
	echo "File  ${DATFILE}  does not exist!"
fi
if [[ -f $SETFILE ]];then
	echo "File  ${SETFILE}  does not exist!"
fi

fi ##END CYCLEING IF STATEMENT

done
