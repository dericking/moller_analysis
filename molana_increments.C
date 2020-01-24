#define molana_scalers_cxx
#include<TROOT.h>
#include<TH2.h>
#include<TSystem.h>
#include<TStyle.h>
#include<TCanvas.h>
#include<TFile.h>
#include<TTree.h>
#include<TBranch.h>
#include<sstream>
#include<iostream>

//TODO: THIS SHOULD BE FORMALIZED BETTER. PERFORM INCREMENT CALCULATIONS IN 50K ENTRY BATCHES
//      WITH A FILLINCREMENTSROOTFILE() FUNCTION THAT GETS CALLED EVERY 50K ENTRIES.

Bool_t isnonnum(char c){
  return !(c >= '0' && c <= '9');
}

void molana_increments(string FILE, const Int_t DELAY)
{
  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //CHECK FOR THE EXISTENCE OF ENVIRONMENTAL VARIABLES
  if(!gSystem->Getenv("MOLLER_ROOTFILE_DIR")){
    cout <<"Set environment variable MOLLER_ROOTFILE_DIR\n"
         <<"to point at the directory where the .root files\n"
         <<"should be stored then rerun."<<endl;
    exit(0);
  }


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //STRIP RUN NUMBER FROM FILE NAME
  std::string fnametrun = FILE.substr( FILE.find_last_of("\\/")+1 );
  std::string fnamecopy = fnametrun;
  fnamecopy.erase(std::remove_if(fnamecopy.begin(),fnamecopy.end(),isnonnum),fnamecopy.end());
  stringstream ss;
  ss << fnamecopy;
  Int_t RUNN;
  ss >> RUNN;


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //LOAD ROOT FILE
  TString sfile("");
  sfile=FILE;
  cout << "gSystem->AccessPathName( " << sfile << " ): " << gSystem->AccessPathName( sfile ) << endl;
  if( (gSystem->AccessPathName( sfile )) ){  // RETURNS: FALSE IF ONE CAN ACCESS FILE, TRUE IF YOU CAN'T 
    cout << "molana_increments.C() ==> ROOT data not found" << endl;
    exit(0);
  } else {
    cout << "molana_increments.C() ==> ROOT data file found" << endl;
  }
  TString sizecmd("");
  sizecmd = Form("wc -c %s/moller_data_%d.root | awk '{print $1}'", gSystem->Getenv("MOLLER_ROOTFILE_DIR") ,RUNN);
  TString datafilesizestring = gSystem->GetFromPipe( sizecmd );
  Int_t datafilesizeint      = datafilesizestring.Atoi();
  if(datafilesizeint < 75000){
    cout << "molana_increments.C() ==> ROOT data file extremely small. Conversion error maybe? Exiting... " << endl;
    exit(0);
  }
  TFile * fin = new TFile( sfile );
  TTree * trSca;
  fin->GetObject("trSca",trSca);
  Long64_t nentries = trSca->GetEntries();
  cout << "molana_increments.C() ==> Total number of entries in DATA file is: " << nentries << endl;
  const Int_t N = nentries;

  cout << "molana_increments.C() ==> Declaring data holders." << endl;
  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DATA HOLDERS
  const Int_t     NSCA = 32;
  const Int_t     NTRG = 8; 
  Short_t         irun;
  Short_t         idtype;
  Short_t         ievtype;
  Int_t           iret;
  Double_t        unix_time;
  Int_t           ntrig;
  Short_t         itrig[NTRG];
  Int_t           itick;
  Int_t           nsca;
  Int_t           isca[NSCA];

  Int_t allscal[N][NSCA];
  Int_t alltrig[N][NTRG];
  Int_t alltick[N];

  //Double_t alltimes[nentries];//Occassionally tossing a core-dump due to some sort of memory addressing problem. Happened in 9 out of 1400 runs.  :/
  Int_t * alltimes = NULL;
  alltimes = new Int_t [nentries];

  Int_t prevscal[NSCA];
  Int_t prevtrigs[NTRG];
  Int_t prevtick = 0;
  Int_t prevunixtime = 0;

  for(Int_t i = 0; i < N; i++){
    alltick[i] = 0;
    for(Int_t j = 0; j < NSCA; j++) allscal[i][j] = 0;
    for(Int_t j = 0; j <  NTRG; j++) alltrig[i][j] = 0;
  }
  for(Int_t j = 0; j < NSCA; j++) prevscal[j] = 0;
  for(Int_t j = 0; j <  NTRG; j++) prevtrigs[j] = 0;

  Int_t inccounter = 0;

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //ISCA CHANNEL NAMES - CAN BE READ BY CONFIGURATION FILE LATER ON.
  Char_t scalernames[NSCA][32] = {"singl1",
                                "singr1",
                                "coinc1",
                                "accid1",
                                "bcm",
                                "ledflash",
                                "sca6",
                                "sca7",
                                "sca8",
                                "clockheltrig",/*clock * helicity*/
                                "mps",
                                "clockheldel",
                                "sca12",
                                "sca13",
                                "clocktimer",
                                "sca15",
                                "singl2",
                                "singr2",
                                "coinc2",
                                "accid2",
                                "detsumleft",
                                "detsumright",
                                "detsumboth",
                                "appleft",
                                "appright",
                                "bpm1h01_pxm",
                                "bpm1h01_pxp",
                                "bpm1h01_mxm",
                                "bpm1h01_mxp",
                                "sca29",
                                "sca30",
                                "sca31"};
  Char_t trignames[NTRG][16] =    {"trig0",
                                "trig1",
                                "trig2",
                                "trig3",
                                "trig4",
                                "trighelvalue",
                                "trig6",
                                "trighelpatt" };


  cout << "molana_increments.C() ==> Assigning branch addresses for molana_data" << endl;

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //TREE BRANCHES
  TBranch        *b_irun;
  TBranch        *b_idtype;
  TBranch        *b_ievtype;
  TBranch        *b_iret;
  TBranch        *b_ntrig;
  TBranch        *b_itrig;
  TBranch        *b_itick;
  TBranch        *b_nsca;
  TBranch        *b_isca;
  TBranch        *b_unix_time;
  trSca->SetBranchAddress(     "irun",      &irun, &b_irun);
  trSca->SetBranchAddress(   "idtype",    &idtype, &b_idtype);
  trSca->SetBranchAddress(  "ievtype",   &ievtype, &b_ievtype);
  trSca->SetBranchAddress(     "iret",      &iret, &b_iret);
  trSca->SetBranchAddress(    "ntrig",     &ntrig, &b_ntrig);
  trSca->SetBranchAddress(    "itrig",      itrig, &b_itrig);
  trSca->SetBranchAddress(    "itick",     &itick, &b_itick);
  trSca->SetBranchAddress(     "nsca",      &nsca, &b_nsca);
  trSca->SetBranchAddress(     "isca",       isca, &b_isca);
  trSca->SetBranchAddress("unix_time", &unix_time, &b_unix_time);

  cout << "molana_increments.C() ==> Starting reading molana_data file..." << endl;

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //CYCLE THROUGH THE DATA
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t entry = 0; entry < nentries; entry++) {
    nb = trSca->GetEntry(entry);
    nbytes += nb;

    Bool_t b_trigrecorded = false;
    Bool_t b_scalersrecorded = false;

    if(nsca == 32){//DOUBLE CHECK THIS FOR NOW
      for(Int_t i = 0; i < nsca; i++){
        Int_t increment = isca[i] - prevscal[i];
        allscal[entry][i] = increment;
        prevscal[i] = isca[i];
      }
      b_scalersrecorded = true;
    }

    if(ntrig == 8){//DOUBLE CHECK THIS FOR NOW
      for(Int_t i = 0; i < ntrig; i++) alltrig[entry][i] = itrig[i];
      b_trigrecorded = true;
    }

    alltick[entry] = ( itick - prevtick );
    alltimes[entry] = unix_time;
    prevtick = itick;
    prevunixtime = unix_time;

    if(b_scalersrecorded && b_trigrecorded) inccounter++;
  }

  cout << "molana_increments.C() ==> Starting writing molana_increments file..." << endl;

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //GREAT! NOW WE HAVE ALL THE INCREMENTS CALCULATED - REMEMBER THE HELICITY REPORTED IS DELAYED
  TFile * fout = new TFile( Form("%s/molana_increments_%i.root",gSystem->Getenv("MOLLER_ROOTFILE_DIR"),RUNN) , "RECREATE");
  TTree * trInc = new TTree( "trInc" , Form("Increment Tree Run %d",RUNN) );

  Int_t temparrscal[32];
  Int_t temparrtrig[32];
  Int_t tempinttick;
  Int_t tempintunixtime;

  for(Int_t i = 0; i < 32; i ++)
  trInc->Branch( scalernames[i] , &temparrscal[i] , Form( "%s/I",scalernames[i] ) );
  for(Int_t i = 0; i < 8; i ++)
  trInc->Branch( trignames[i]   , &temparrtrig[i] , Form( "%s/I",trignames[i]   ) );
  trInc->Branch( "ticks"        , &tempinttick    , "ticks/I"                     );
  trInc->Branch( "unixtime"     , &tempintunixtime, "unix_time/I"                 );

  for(Int_t n = (1 + DELAY); n < (const Int_t)inccounter-DELAY; n++){
    for(Int_t i = 0; i < 32; i ++){
        if(i!=9){
          temparrscal[i] = allscal[n][i];
        }else{
          //HELICITY DELAY
          temparrscal[i] = allscal[n+DELAY][i];
        }
    }
    for(Int_t i = 0; i <  8; i ++){
      if(i!=5){
        temparrtrig[i] = alltrig[n][i];
      }else{
        //HELICITY DELAY
        temparrtrig[i] = alltrig[n+DELAY][i];
      }
    }
    tempinttick = alltick[n];
    tempintunixtime = alltimes[n];

    trInc->Fill();
  }

  fout->Write();
  cout << "molana_increments.C() ==> MOLANA increments ROOT file created..." << endl;

}

