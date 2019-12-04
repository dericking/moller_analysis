# moller_analysis
## 2019 version
Starting repository for moller polarimetry analysis. We can transfer this to Simona once it's usable.

### How to use:
If analysis for a single run is desired     *./run_analysis.sh N*
If analysis for a batch of runs is desired  *./run_analysis.sh N1 N2*

Currently, files for which moller_data_NNNNN.nt does not exist are skipped over. This is reasonable.

*run_analysis.sh* will check for the existence of the root file first in **moller_data_temp** and if the ROOT file does not exist there it will be searched for by *copy_moller_data.sh* which will pull the n-tuple file and the settings file into the **moller_data_temp** directory; *run_analysis.sh* will then run *h2root* on the n-tuple converting it to a ROOT file, perform the analysis *eric_asym.C* and dump the results into the specified **analysis** folder.

I guess it's worth stating but my intent is to eventually use this without having to go through ROOT. Let's call that Moller_Analysis v2.0. I want to create a main(), move things to methods if needed, and allow the program to take arguments such as anpow and compile with g++.
### Rules:
Commit as you please and how you please. Feel free to add to-do lists, thoughts, ideas under *Issues*.
