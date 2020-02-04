# __MOLANA Analysis (2020 version)__
Repository for **MOLANA** Analysis code. This code is to be used in conjunction with Don Jones' **MOLANA** [raw data decoder](https://github.com/jonesdc76/moller_analyser). This version of the code is intended to be used with a persistent database that maintains prompt results.

## How to use run\_molana\_analysis.sh
The sript *run_molana_analysis.sh* works out all the required coordination between ROOT macros, bash scripts and MOLANA decoder program.

### Getting Started
Helpful for the usage of *run\_molana\_analysis* can be accessed by running the bash script with the _--help_ option.

There exists a persistent configuration file that currently only contains the following information: 

* Analyzing Power
* Charge Pedestal
* Target Polarization

More may be added at some point.

You can check to see if the configuration file is available by running the script with the _--checkconfig_ option; if nothing prints on the screen you can create a cofiguration file template running the script with the _--createconfig_ option. There will be (or perhaps now are) options to then set these values from the command line with the *run\_molana\_analyis.sh* script. The values this file pulls will be the values used for the last successful prompt data analysis that was sent to the database.

For example: 

 *run\_molana\_analysis.sh* *--anpow 0.75309 --targpol 0.08012 --qped 9*

Will (when done) fix those values in the persistent configuration file.

### Analysis

 *run\_molana\_analysis.sh* *--run 18915*

The above command will analyze run #18915. 
* __IF__ run #18915 is unlisted in the database the script will automatically default to the configuration file values for analysis.
* __IF__ run #18915 is listed in the database the script will simply re-analyze with the old specified values.

*run\_molana\_analysis.sh* *--run 18915 --forcecfg*

The above command will force all of the new configuration settings on the analysis. You can also choose to force only one of the values (*see --help*).

*run\_molana\_analysis.sh* *--batchstart 18915 --batchend 18920*

The above command will analyze all runs in the specified span. Again, if they are new runs they will default to the configuration specified __OR__ if they are old runs they will re-analyze against the previously provided parameter.

*run\_molana\_analysis.sh* *--batchstart 18915 --batchend 18920 --forcecfg*

The above command will force the batch run to use the new specified parameters.

### Bleedthrough Runs

Bleedthrough runs should be handled as follows:

*run_molana_analysis.sh --run 18915 --bleed*

Bleedthough results will be written to the database of prompt analysis results.

## Using molana_increments.C()

*void molana_increments(string FILE, const Int_t DELAY)*

Molana increments root macro takes the converted root file of the raw data decoded by Don Jones' MOLANA raw decoder. The macro takes two arguements: Location of the file to be analyzed; and helicity delay. Molana increments calculates up the increments between helicity cycles, removes the helicity delay between the helicity patter and the data and writes a new file called *molana_increments_NNNNN.root* which can then be read by the molana analysis script.

## Using molana_analysis.C()

*molana_analysis(string FILE, Int_t HELN, Double_t FREQ, Double_t ANPOW, Double_t QPED )*

Molana analysis root macro takes 5 arguments: Location of the *molana_increments_NNNNN.root* file to be read; helicity pattern type; helicity frequency; analyzing power; and charge pedestal. This can be run on its own in standard ROOT fashion. When run as standalone the script will produce PNG files and an errors text file in the directory which it resides. The script will push prompt analysis results to the specified database.

## Using molana_bleedthrough.C()

molana_bleedthrough(string FILE, Double_t FREQ, Bool_t BEAM )

Molana bleedthrough root macro takes 3 arguments: File location; helicity frequency; and beam status. It analyzes, on a helicity cycle basis, both charge pedestal and bleedthrough rates. When beam is off [BEAM = 0] the script proceeds guaranteed that the beam was not on and spits out *charge pedestal* **and** *bleedthrough rates*. When beam is on [BEAM = 1] the script looks for times when beam is off on the conditions: *bcm < (bcm_mean - 5 sigma)* **and** *coin < (coin_mean - 3 sigma)*. This has produced reliable results of finding beam trips from which the *charge pedestal* is calculated.

# Important information: 

These scripts require the use of the following environmental variables:

* MOLLER\_ROOTFILE\_DIR   (Also required by the MOLANA raw data decoder)
* MOLLER\_DATA\_DIR       (Also required by the MOLANA raw data decoder)

* MOLANA\_DB\_HOST (Used by *run_molana_analysis.sh*, *molana_analysis.C* and *molana_bleedthrough.C* )
* MOLANA\_DB\_USER (Used by *run_molana_analysis.sh*, *molana_analysis.C* and *molana_bleedthrough.C* )
* MOLANA\_DB\_PASS (Used by *run_molana_analysis.sh*, *molana_analysis.C* and *molana_bleedthrough.C* )
* MOLANA\_DB\_NAME (Used by *run_molana_analysis.sh*, *molana_analysis.C* and *molana_bleedthrough.C* )

* MOLLER\_SETTINGS\_DIR      (Where does the DAQ throw the *mollerrun_NNNNN.set* files?)
* MOLANA\_DATADECODER\_DIR   (Where is Don Jones' MOLANA raw decoder located)
* MOLANA\_ONLINE\_PUSH\_DIR  (Where should *run_molana_analysis.sh* push PNG results to?)
