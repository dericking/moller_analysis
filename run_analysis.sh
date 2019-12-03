#!/bin/bash

#Need if running on moller account
#source /adaqfs/apps/ROOT/pro/bin/thisroot.csh

#Where to store the results files.
#RSLTDIR="/u/group/halla/www/hallaweb/html/equipment/moller/eric_asym/analysis"
RSLTDIR="analysis"
[ -d "${RSLTDIR}" ] && echo "Moller online analysis directory exists. Woohoo!!!" || mkdir ${RSLTDIR}
#does the files and date folder in the results directory exist?
[ -d "${RSLTDIR}/files" ] && echo "Moller online analysis directory exists. Woohoo!!!" || mkdir "${RSLTDIR}/files"
[ -d "${RSLTDIR}/date" ] && echo  "Moller online analysis directory exists. Woohoo!!!" || mkdir "${RSLTDIR}/date"
#Where to find the data files
FILEDIR="moller_data_temp"
[ -d "${FILEDIR}" ] && echo  "Temporary directory for moller_data_NNNNN.nt and ROOT files exists. Nice!" || mkdir "${FILEDIR}"


rm -f *.png
rm -f *.pdf
rm -f summary.txt
touch summary.txt

START=$1
if [[ -z ${2} ]]; then
    END=$1
else
    END=${2}
fi

echo "Starting analysis for ${START} to ${END}"

for (( ANALRUN=$START; ANALRUN<=$END; ANALRUN++ )); do
    
    #Check to see if the ntuple has already been converted to a root file. If so... nothing needed. Else, copy and convert.
    if [[ -f "${FILEDIR}/moller_data_${ANALRUN}.root" && -f "${FILEDIR}/mollerrun_${ANALRUN}.set" ]]; then
        echo "Needed data files exist..."
    else
        echo "Fetching needed datafiles..."
        ./copy_moller_data.sh ${FILEDIR} ${ANALRUN} ${ANALRUN}
        h2root ${FILEDIR}/moller_data_${ANALRUN}.nt  
    fi

    echo "Analyzing run ${ANALRUN}"

    SETFILE="${FILEDIR}/mollerrun_${ANALRUN}.set"
    DATFILE="${FILEDIR}/moller_data_${ANALRUN}.root"

    echo "settings file location: ${SETFILE}"
    echo "rootdata file location: ${DATFILE}"

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
        FREQNCY=${FREQNCY//[!0-9.]/}
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

        root -b -l -q "eric_asym.C+(\""${DATFILE}"\","${PATTERN}","${HELDLAY}","${FREQNCY}")"

        [ ! -d "${RSLTDIR}/files" ]            || echo "[${RSLTDIR}/files] Does not exist!!!" 
        [ ! -d "${RSLTDIR}/files" ]            && mkdir ${RSLTDIR}/files
        [ ! -d "${RSLTDIR}/files/${ANALRUN}" ] || echo "[${RSLTDIR}/files/run_${ANALRUN}] Does	not exist!!!"
        [ ! -d "${RSLTDIR}/files/${ANALRUN}" ] && mkdir ${RSLTDIR}/files/run_${ANALRUN}

        mv  analysis_${ANALRUN}.pdf    ${RSLTDIR}/files/run_${ANALRUN}/
        cp  final_stats_${ANALRUN}.txt ${RSLTDIR}/files/run_${ANALRUN}/
        mv  errors_${ANALRUN}.txt      ${RSLTDIR}/files/run_${ANALRUN}/
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

