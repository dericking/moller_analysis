#!/bin/bash

CFGFILE="molana_analysis_configuration.dat"

##COPY LARGER ENV VAR NAMES TO COMPACT
MDBHOST=${MOLANA_DB_HOST}
MDBUSER=${MOLANA_DB_USER}
MDBPASS=${MOLANA_DB_PASS}
MDBNAME=${MOLANA_DB_NAME}
MROOTDR=${MOLLER_ROOTFILE_DIR}
ANALDIR=${MOLANA_DATADECODER_DIR}
RSLTDIR=${MOLANA_ONLINE_PUSH_DIR}

BATCH=           #Are we running in batch mode?
START=           #Start analysis at [run]
ENDAT=           #End analysis at [run]
FORCE=false      #Force configuration settings over used database settings
ISBLEED=         #Is it a bleedthrough run?
MAKENEW=false    #Make new molana files?
ANALPOW=0.77777  #Default anpow if not in database and not forced to config
TARGDEF=0.08010  #Default tarpol if not in database and not forced to config ... there should probably be a line for this in moller_run_details to cross-check.
CHRGPED=0        #Default qped if notin database and not forced to config ... there should probably be a line for this in the moller_run_details to cross-check.
FORCEAP=false    #Force analyzing power
FORCEQP=false    #Force charge pedestal
FORCETP=false    #Force target polarization

function printhelp(){

echo -e "     -r | --run      )  Sets start and end values for a single run analysis.\n"

echo -e "     -b | --bleed    )  Explicity forces bleedthrough analysis on runs where beam is "
echo -e "                        assured as off.\n"

echo -e "     -f | --forcecfg )  Forces analysis to use ALL configuration file values rather than "
echo -e "                        values on database record. This should be used if we want to "
echo -e "                        update analysis with new values. These values WILL REPLACE "
echo -e "                        values used in the database.\n"

echo -e "     --forceanpow    )  Forces only the analyzing power specified in the molana "
echo -e "                        configuration file to be used. Other values will be taken "
echo -e "                        from the database history. If the database contains a NULL or "
echo -e "                        empty value the script will default to the configuration file.\n"

echo -e "     --forceqped     )  Forces only the charge pedestal specified in the molana "
echo -e "                        configuration file to be used. Other values will be taken "
echo -e "                        from the database history. If the database contains a NULL or "
echo -e "                        empty value the script will default to the configuration file.\n"

echo -e "     --forcetargpol  )  Forces only the target polarization specified in the molana "
echo -e "                        configuration file to be used. Other values will be taken "
echo -e "                        from the database history. If the database contains a NULL or "
echo -e "                        empty value the script will default to the configuration file.\n"

echo -e "     -n | --newfile  )  Forces creation of new molana data/increments files. Whould be"
echo -e "                        used if the Molana reader or the molana increments file structure"
echo -e "                        needs to be updated.\n"

echo -e "     --createconfig  )  Quickly creates a configuration file [molana.cfg] based on the "
echo -e "                        most recent successfully analyzed run if the file does not exist.\n"

echo -e "     --checkconfig   )  Will print your configuration file on the screen for quick analysis. "
echo -e "                        If this pops up blank then run --createconfig for new config file.\n"

echo -e "     -a | --anpow    )  Run as --anpow='0.xxxx' to replace the value after the equals sign"
echo -e "                        in the configuration file. You could also do this by hand via"
echo -e "                        something like nano or gedit if you desired.\n"

echo -e "     -q | --qped     )  Run as --qped='0.xxxx' to replace the value after the equals sign"
echo -e "                        in the configuration file. You could also do this by hand via"
echo -e "                        something like nano or gedit if you desired.\n"

echo -e "     -p | --targpol  )  Run as --targpol='0.xxxx' to replace the value after the equals sign"
echo -e "                        in the configuration file. You could also do this by hand via"
echo -e "                        something like nano or gedit if you desired.\n"

echo -e "     -c | --comment  )  TODO: Will insert small comment into the database for the runs being"
echo -e "                        analyzed. This will most likely be a VARCHAR(48) and part of cfg file."
echo -e "                        Should be run as --comment='short comment with no commas'."

echo -e "     --batchstart    )  Sets BATCH=true and assigns a batch start number. Start number"
echo -e "                        must be smaller than end number. Failure to enter both --batchstart "
echo -e "                        and --batchend will result in an exit.\n"

echo -e "     --batchend      )  Sets BATCH=true and assigns a batch start number. Start number"
echo -e "                        must be smaller than end number. Failure to enter both --batchstart "
echo -e "                        and --batchend will result in an exit.\n"

echo -e "     -h | --help     )  Prints the help. Duh."

exit;

}

function setforce(){
    FORCE=true;
}
function setforceap(){
    FORCEAP=true;
}
function setforceqp(){
    FORCEQP=true;
}
function setforcetp(){
    FORCETP=true;
}

function checkconfig(){
    if [[ -f "${CFGFILE}" ]]; then
        echo " ";
        cat ${CFGFILE}
        echo -e "\n${CFGFILE} file exists.";
    else
        echo -e "\nWARNING! ${CFGFILE} does not exist.";
        echo -e "Run with option --createconfig to create new molana_analysis.cfg file.";
        echo -e "After which you can run a command to change a value or edit the file by hand.\n";
    fi
    exit;
}

function createconfig(){
    rm -f ${CFGFILE};
    local LASTRUNN=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT id_run FROM moller_run WHERE run_anpow IS NOT NULL AND run_qped_used IS NOT NULL AND run_ptarg > 0. AND run_ptarg IS NOT NULL ORDER BY id_run DESC LIMIT 1;")
    local LASTANPW=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_anpow FROM moller_run WHERE id_run = ${LASTRUNN};")
    local LASTQPED=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_qped_used FROM moller_run WHERE id_run = ${LASTRUNN};")
    local LASTPTAR=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_ptarg FROM moller_run WHERE id_run = ${LASTRUNN};")
    CFLINE1="CFGAPOW=${LASTANPW}";
    CFLINE2="CFGQPED=${LASTQPED}";
    CFLINE3="CFGPTAR=${LASTPTAR}";
    echo -e "${CFLINE1}\n${CFLINE2}\n${CFLINE3}" >> ${CFGFILE};
    exit;
}

function changeconfig(){
    case "$1" in
    'CFGAPOW')
    echo "Changing ${1} to ${2}"
    ;;
    'CFGQPED')
    echo "Changing ${1} to ${2}"
    ;;
    'CFGPTAR')
    echo "Changing ${1} to ${2}"
    ;;
    esac
}

function validatebatch(){
    if [[ -z "${START}" || -z "${ENDAT}" ]]; then
        echo -e "Error. Bad batch setup. Starting run and/or ending run is NULL. Exiting... \n"
        exit;
    fi
    if [[ ${START} > ${ENDAT} ]]; then
        echo -e "Error. Starting run is GREATER THAN ending run. Learn to count. :) Exiting... \n"
        exit;
    fi
}


while true; do
  case "$1" in
    -r | --run      )  START=$2; ENDAT=$2; shift 2 ;;
    -b | --bleed    )  ISBLEED=true; shift ;;
    -f | --forcecfg )  setforce; shift ;;
    -n | --newfile  )  MAKENEW=true; shift ;;
    -a | --anpow    )  changeconfig "CFGAPOW" $2; shift 2 ;;
    -q | --qped     )  changeconfig "CFGQPED" $2; shift 2 ;;
    -p | --targpol  )  changeconfig "CFGPTAR" $2; shift 2 ;;
    -c | --comment  )  changeconfig "comment" $2; shift 2 ;;
    --checkconfig   )  checkconfig; shift ;;
    --createconfig  )  createconfig; shift ;;
    --batchstart    )  BATCH=true; START=$2; shift 2 ;;
    --batchend      )  BATCH=true; ENDAT=$2; shift 2 ;;
    --forceanpow    )  setforceap; shift ;;
    --forceqped     )  setforceqp; shift ;;
    --forcetargpol  )  setforcetp; shift ;;
    --help          )  printhelp; shift;;
    --              )  shift; break ;;
    *               )
                    if [[ -z "${1}" ]]; then
                        echo "";
                    else
                        echo "$1 is not an option. You need --help";
                    fi
                    break;; 
  esac
done


#MAKE SURE THAT BATCH VALUES ARE ACCEPTABLE
if [[ "${BATCH}" == "true" ]]; then
  validatebatch;
fi

#MAKE SURE THESE DIRECTORIES EXIST
[ -d "${RSLTDIR}" ]       && echo "run_molana_analysis() ==> Specified analysis repository exists." || mkdir "${RSLTDIR}"
[ -d "${RSLTDIR}/files" ] && echo "run_molana_analysis() ==> Specified analysis files directory exists." || mkdir "${RSLTDIR}/files"
[ -d "${MROOTDR}/prompt_stats" ] && echo "run_molana_analysis() ==> Specified directory for Prompt Results exists." || mkdir "${MROOTDR}/prompt_stats"

#REMOVE ANY OUTPUT FILES WHICH MAY BE LINGERING
rm -f *.txt
rm -f *.png
rm -f *.pdf

echo "run_molana_analysis() ==> Starting analysis for ${START} to ${ENDAT}"

####################################################################################################################################
##exit;

for (( ANALRUN=${START}; ANALRUN<=${ENDAT}; ANALRUN++ )); do
    ##WHERE ARE THE FILES THAT WE NEED LOCATED?
    SETFILE="${MROOTDR}/mollerrun_${ANALRUN}.set"
    DATFILE="${MROOTDR}/moller_data_${ANALRUN}.root"
    INCFILE="${MROOTDR}/molana_increments_${ANALRUN}.root"
    ##HAVE WE ALREADY COPIED THE SETTINGS FILE?
    if [[ -f "${SETFILE}" ]]; then
        echo "run_molana_analysis() ==> Settings file already copied to MOLANA ROOT repository. :)"
    else
        ./run_copy_moller_settings.sh ${ANALRUN} ${ANALRUN}
        ./populate_settings_in_molpol_db.sh ${ANALRUN}
    fi

    echo "run_molana_analysis() ==> **********MAKENEW: ${MAKENEW}***********"
    echo "run_molana_analysis() ==> **********ISBLEED: ${ISBLEED}***********"

    ##HAVE WE ALREADY CONVERTED THE RAW DATA TO THE MOLANA DATA ROOT FILE?
    if [[ -f "${DATFILE}" && "${MAKENEW}" == false ]]; then
        echo "run_molana_analysis() ==> Needed MOLANA data file exists..."
    else
        echo "run_molana_analysis() ==> Making MOLANA data file..."
        ${ANALDIR}/molana ${ANALRUN}
    fi

    ##IF WE HAVE THE SETTINGS AND DATA FILE LET'S START
    if [[ -f $SETFILE && -f $DATFILE ]];then
        ##WHAT IS THE PATTERN
        PATTERN=$(sed -n -e "s#\(Helicity Mode ON/OFF Random/Toggle   HELPATTERNd          : \)##p" $SETFILE)
        if [ $PATTERN == "Octet" ]; then
          PATTERN=8;
        fi
        if [ $PATTERN == "Quartet" ]; then
          PATTERN=4;
        fi
        ##WHAT IS THE FREQUENCY? DELAY?  DATE?
        FREQNCY=$(sed -n -e "s#\(Helicity frequency                   HELFREQ              : \)##p" $SETFILE)
        HELDLAY=$(sed -n -e "s#\(Helicity delay                       HELDELAYd            : \)##p" $SETFILE)
        FREQNCY=${FREQNCY//[!0-9.]/}
        HELDLAY=${HELDLAY//[!0-9]/}
        RUNDATE=$(sed -n -e "s#\(Date       : \)##p" $SETFILE)
        read dayname month datnum clock zone year <<< ${RUNDATE}
        RUNDATE=$(date --date="$(printf "${RUNDATE}")" +%Y-%m-%d)
        RUNTIME=$(date --date="$(printf "${RUNDATE}")" +%H-%M-%S)


        ##HAVE WE ALREADY CREATED THE MOLANA INCREMEMENTS FILE? OR DO WE WANT TO DO IT FRESH?
        if [[ -f ${INCFILE} && "${MAKENEW}" == false ]]; then
            echo "run_molana_analysis() ==> MOLANA Increments ROOT file already exists! :)"
        else
            echo "run_molana_analysis() ==> Making MOLANA increments ROOT file..."
            root -b -l -q "molana_increments.C+(\""${DATFILE}"\","${HELDLAY}")"
        fi

        ##IS THE RUN NEW TO THE DATABASE? IN WHICH CASE WE MUST FORCE THE CONFIGURATION FILE
        ISOLDRUN=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT id_run FROM moller_run WHERE id_run = ${ANALRUN};")
        if [[ -z "$ISOLDRUN" ]]; then FORCE=true; fi

        ##IF NOT FORCING VALUES GET THEM FROM DB
        if [[ -z "${FORCE}" || "${FORCE}"="false" ]]; then
            ANPOWER=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_anpow FROM moller_run WHERE id_run=${ANALRUN};")
            echo "run_molana_analysis() ==> Returned ANPOWER from hamolpol moller_run table is: ${ANPOWER}"
            if [[ -z "$ANPOWER" || "$ANPOWER" == "NULL" ]]; then
               ANPOWER=${ANALPOW} #OLD RUN RETURNS NO ANALYSING POWER ASSIGNED... USE DEFAULT TO FIND EASILY IN DATABASE AND CORRECT
            fi
            CHRGPED=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_qped_used FROM moller_run WHERE id_run = ${ANALRUN};")
            if [[ -z "$CHRGPED" || "$CHRGPED" == "NULL" ]]; then
               CHRGPED=0
            fi
            TARGPOL=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT run_ptarg FROM moller_run WHERE id_run = ${ANALRUN};")
            if [[ -z "$TARGPOL" || "$TARGPOL" == "NULL" ]]; then
               TARGPOL=${TARGDEF}
            fi
        fi

        . ${CFGFILE};

        ##IF FORCE FORCEANPOW FORCEQPED OR FORCEPTARG
	if [[ "${FORCEAP}" == true || "${FORCE}" == true ]]; then ANPOWER=${CFGAPOW}; fi
	if [[ "${FORCEQP}" == true || "${FORCE}" == true ]]; then CHRGPED=${CFGQPED}; fi
	if [[ "${FORCETP}" == true || "${FORCE}" == true ]]; then TARGPOL=${CFGPTAR}; fi

        ##ECHO THE VALUES OBTAINED
        echo "run_molana_analysis() ==> Data file: ${DATFILE}"
        echo "run_molana_analysis() ==> H.Pattern: ${PATTERN}"
        echo "run_molana_analysis() ==> Frequency: ${FREQNCY}"
        echo "run_molana_analysis() ==> Delay Num: ${HELDLAY}"
        echo "run_molana_analysis() ==> Date/Time: ${RUNDATE}"
        echo "run_molana_analysis() ==> AnalPower: ${ANPOWER}"
        echo "run_molana_analysis() ==> ChargePed: ${CHRGPED}"
        echo "run_molana_analysis() ==> TargetPol: ${TARGPOL}"

        ##IS TYPE OF RUN IN MOLLER_RUN_DETAILS? IF NOT LABEL IT "PENDING". TODO: WAS BLEEDTHROUGH FLAG (--bleed) PASSED?
	RUNTYPE=$(mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "SELECT rundet_type FROM moller_run_details WHERE id_rundet=${ANALRUN};")
        if [[ -z "$RUNTYPE" || "$RUNTYPE" == "NULL" ]]; then
            ##IF IT WASN'T FORCED AS A BLEED_THROUGH LABEL IT PENDING
            if [[ "${ISBLEED}" == true ]]; then 
                RUNTYPE="bleed_through"; 
            else
                RUNTYPE="pending"
            fi
            ##PUSH THE RUN TYPE TO THE PREX RUN_DETAILS PAGE
            mysql -h ${MDBHOST} --user="${MDBUSER}" --password="${MDBPASS}" --database="${MDBNAME}" --skip-column-names -e "INSERT IGNORE INTO moller_run_details (id_rundet,rundet_day,rundet_type) VALUES (${ANALRUN},'${RUNDATE}','${RUNTYPE}');"
        fi

        ##RUN ANALYSIS
        if [[ "$RUNTYPE" == "bleed_through" || "${ISBLEED}" == true ]]; then
          echo "run_molana_analysis() ==> Running molana_bleedthrough..."
          root -b -l -q "molana_bleedthrough.C+(\""${INCFILE}"\",${FREQNCY},0)"
        else
          root -b -l -q "molana_analysis.C+(\""${INCFILE}"\","${PATTERN}","${FREQNCY}","${ANPOWER}","${CHRGPED}")"
        fi
      
        ##DOES THE DIRECTORY FOR THE RUN EXIST FOR THE OUTPUT IMAGES?
        if [ ! -d "${RSLTDIR}/files/run_${ANALRUN}" ]; then mkdir ${RSLTDIR}/files/run_${ANALRUN}; fi
        rm -f ${RSLTDIR}/files/run_${ANALRUN}/*.png
        rm -f ${RSLTDIR}/files/run_${ANALRUN}/*.pdf
        mv  errors_${ANALRUN}.txt          ${RSLTDIR}/files/run_${ANALRUN}/
        mv  *.png                          ${RSLTDIR}/files/run_${ANALRUN}/
        mv  molana_*_stats_${ANALRUN}      ${MROOTDR}/prompt_stats/

        ##DO WE WANT TO LEAVE A FILE RECORD OF THE MOLLER CONFIGURATIONS. SIZE IS SMALL. COULD BE USED IN CASE OF DB FAILURE... IMPLEMENT LATER

    else
        if [[ -f $DATFILE ]];then
            echo "run_molana_analysis() ==> File  ${DATFILE}  does not exist!"
        fi
        if [[ -f $SETFILE ]];then
            echo "run_molana_analysis() ==> File  ${SETFILE}  does not exist!"
        fi
    fi

done

./run_print_prompt_stats.sh

