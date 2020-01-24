# __MOLANA Analysis (2020 version)__
Repository for MOLANA Analysis code.

## How to use:
The sript *run_molana_analysis.sh* works out all the required coordination between ROOT macros, bash scripts and MOLANA decoder program.

### *run_molana_analysis.sh*

#### Getting Started
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

#### Analysis

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

#### Bleedthrough Runs

Bleedthrough runs should be handled as follows:

*run_molana_analysis.sh* *--run 18915 --bleed*

### molana_increments.C()

Update with brief explanation of ROOT script.

### molana_analyssis.C()

Update with brief explanation of ROOT script.

### molana_bleedthrough.C()

Update with brief explanation of ROOT script.

## Important information: 

These scripts require the use of the following environmental variables:

* MOLLER\_ROOTFILE\_DIR (Also required by the MOLANA raw data decoder)
* MOLLER\_DATA\_DIR (Also required by the MOLANA raw data decoder)

* MOLANA\_DB\_HOST
* MOLANA\_DB\_USER
* MOLANA\_DB\_PASS
* MOLANA\_DB\_NAME

* MOLLER\_SETTINGS\_DIR
* MOLANA\_DATADECODER\_DIR
* MOLANA\_ONLINE\_PUSH\_DIR
