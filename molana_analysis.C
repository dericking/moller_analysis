//**************************************************************** (╯°□°）╯︵ ┻━┻
//
//  Macro currently takes three arguments
//     (1) FILE -- [Filename as string]
//     (2) HELN -- [4:Quartet,8:Octet]
//     (3) FREQ -- [frequency in hertz]
//     (4) ANPOW - [analyzing power to calculate polarization]
//     (5) QPED  - [charge pedestal]
//
//******************************************************************************

#include<TROOT.h>
#include<TSystem.h>
#include<TFile.h>
#include<TTree.h>
#include<TH1.h>
#include<TH2.h>
#include<TF1.h>
#include<TGraph.h>
#include<TBranch.h>
#include<TString.h>
#include<TCanvas.h>
#include<TPaveStats.h>
#include<TStyle.h>
#include<TSQLServer.h>
#include<vector>
#include<algorithm>
#include<iterator>
#include<iostream>
#include<sstream>
#include<fstream>

Bool_t isnonnumber(char c){
  return !(c >= '0' && c <= '9');
}

void molana_analysis(string FILE, Int_t HELN, Double_t FREQ, Double_t ANPOW, Double_t QPED ){

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //ANALYSIS PARAMETERS
  Bool_t   writetopdf = false;           //SAVE RESULTS ALSO IN PDF
  Bool_t   writetosql = true;            //WRITE RESULTS TO SQL DATABASE
  Bool_t   printascii = false;           //PRINT THE STACKS TO ASCII FILE FOR VISUAL INSPECTION
  const Int_t    heln       = HELN;      //QUARTET(4) OCTET(8)
  const Int_t    stksz      = HELN*10+1; //STACK SIZE 10 CYCLES

  const Double_t tsettle    = 0.000090;       //90 MICROSECOND TSETTLE TIME
  const Double_t freq       = (Double_t)FREQ; //DATA COLLECTION FREQUENCY
  const Double_t gate       = 1./freq-tsettle;//ACTIVE GATE WHILE TAKING DATA
  const Double_t anpow      = ANPOW;          //ANALYZING POWER
  const Double_t ptar       = 0.08012;        //TARGET POLARIZATION
  const Double_t bcmped     = 5;              //BCM PEDESTAL --- TODO: NEED TO FINISH

  Int_t          skippatts  = 1;              //HOW MANY PATTERNS TO SKIP AFTER BAD PATTERN PLUS ONE [i.e. 2 WILL SKIP 1 PATTERN]


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //CHECK FOR THE EXISTENCE OF ENVIRONMENTAL VARIABLES
  if(!gSystem->Getenv("MOLLER_ROOTFILE_DIR")){
    cout <<"Set environment variable MOLLER_ROOTFILE_DIR\n"
         <<"to point at the directory where the .root files\n"
         <<"should be stored then rerun."<<endl;
    exit(0);
  }
  if(!gSystem->Getenv("MOLANA_DB_HOST")){
    cout << "Set environment variable MOLANA_DB_HOST" << endl
         << "to point at the directory where the .root files" << endl
         << "should be stored then rerun." << endl << endl
         << "Turning off write to SQL database functionality." << endl;
    writetosql = false;
  }
  if(!gSystem->Getenv("MOLANA_DB_USER")){
    cout << "Set environment variable MOLANA_DB_USER" << endl
         << "to point at the directory where the .root files" << endl
         << "should be stored then rerun." << endl << endl
         << "Turning off write to SQL database functionality." << endl;
    writetosql = false;
  }
  if(!gSystem->Getenv("MOLANA_DB_PASS")){
    cout << "Set environment variable MOLANA_DB_PASS" << endl
         << "to point at the directory where the .root files" << endl
         << "should be stored then rerun." << endl << endl
         << "Turning off write to SQL database functionality." << endl;
    writetosql = false;
  }
  if(!gSystem->Getenv("MOLANA_DB_NAME")){
    cout << "Set environment variable MOLANA_DB_NAME" << endl
         << "to point at the directory where the .root files" << endl
         << "should be stored then rerun." << endl << endl
         << "Turning off write to SQL database functionality." << endl;
    writetosql = false;
  }

  TString db_host;
  TString db_user;
  TString db_pass;

  if(writetosql){
    db_host = Form("mysql://%s/%s",gSystem->Getenv("MOLANA_DB_HOST"),gSystem->Getenv("MOLANA_DB_NAME"));
    db_user = Form("%s",gSystem->Getenv("MOLANA_DB_USER"));
    db_pass = Form("%s",gSystem->Getenv("MOLANA_DB_PASS"));
  }


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //PRINT OUTPUT
  ofstream output, output2;
  if(printascii) output.open("ascii.out", ios::trunc);
  if(printascii) output2.open("ascii2.out", ios::trunc);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //STRIP RUN NUMBER FROM FILE NAME
  std::string fnametrun = FILE.substr( FILE.find_last_of("\\/")+1 );
  std::string fnamecopy = fnametrun;
  fnamecopy.erase(std::remove_if(fnamecopy.begin(),fnamecopy.end(),isnonnumber),fnamecopy.end());
  stringstream ss;
  ss << fnamecopy;
  Int_t RUNN;
  ss >> RUNN;


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // WRITE DATA STATS TO SQL DATABASE ON START
  if(writetosql){
    TSQLServer* ServerStart = TSQLServer::Connect(db_host,db_user,db_pass);
    TString queryStart(Form("replace into moller_run (id_run) values (%d)",RUNN));
    TSQLResult* resultStart = ServerStart->Query(queryStart.Data());
    queryStart = Form("insert ignore into moller_run_details (id_rundet) values (%d)",RUNN);
    resultStart = ServerStart->Query(queryStart.Data());
    ServerStart->Close();
  }


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //LOAD ROOT FILE
  TString sfile("");
  sfile=FILE;
  cout << "gSystem->AccessPathName( " << sfile << " ): " << gSystem->AccessPathName( sfile ) << endl;
  if( (gSystem->AccessPathName( sfile )) ){  // RETURNS: FALSE IF ONE CAN ACCESS FILE, TRUE IF YOU CAN'T 
    cout << "molana_anaysis.C() ==> ROOT increments file not found" << endl;
    exit(0);
  } else {
    cout << "molana_anaysis.C() ==> ROOT increments file found" << endl;
  }
  TFile * fin = new TFile( sfile );
  TTree * T = nullptr;
  fin->GetObject("trInc",T);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE LEAF TYPES
  Int_t           singl1;
  Int_t           singr1;
  Int_t           coinc1;
  Int_t           accid1;
  Int_t           bcm;
  Int_t           ledflash;
  Int_t           sca6;
  Int_t           sca7;
  Int_t           sca8;
  Int_t           clockheltrig;
  Int_t           mps;
  Int_t           clockheldel;
  Int_t           sca12;
  Int_t           sca13;
  Int_t           clocktimer;
  Int_t           sca15;
  Int_t           singl2;
  Int_t           singr2;
  Int_t           coinc2;
  Int_t           accid2;
  Int_t           detsumleft;
  Int_t           detsumright;
  Int_t           detsumboth;
  Int_t           appleft;
  Int_t           appright;
  Int_t           bpm1h01_pxm;
  Int_t           bpm1h01_pxp;
  Int_t           bpm1h01_mxm;
  Int_t           bpm1h01_mxp;
  Int_t           sca29;
  Int_t           sca30;
  Int_t           sca31;
  Int_t           trig0;
  Int_t           trig1;
  Int_t           trig2;
  Int_t           trig3;
  Int_t           trig4;
  Int_t           trighelvalue;
  Int_t           trig6;
  Int_t           trighelpatt;
  Int_t           ticks;
  Int_t           unixtime;


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE ROOT BRANCHES AND SET ADDRESSES
  TBranch        *b_singl1;
  TBranch        *b_singr1;
  TBranch        *b_coinc1;
  TBranch        *b_accid1;
  TBranch        *b_bcm;
  TBranch        *b_ledflash;
  TBranch        *b_sca6;
  TBranch        *b_sca7;
  TBranch        *b_sca8;
  TBranch        *b_clockheltrig;
  TBranch        *b_mps;
  TBranch        *b_clockheldel;
  TBranch        *b_sca12;
  TBranch        *b_sca13;
  TBranch        *b_clocktimer;
  TBranch        *b_sca15;
  TBranch        *b_singl2;
  TBranch        *b_singr2;
  TBranch        *b_coinc2;
  TBranch        *b_accid2;
  TBranch        *b_detsumleft;
  TBranch        *b_detsumright;
  TBranch        *b_detsumboth;
  TBranch        *b_appleft;
  TBranch        *b_appright;
  TBranch        *b_bpm1h01_pxm;
  TBranch        *b_bpm1h01_pxp;
  TBranch        *b_bpm1h01_mxm;
  TBranch        *b_bpm1h01_mxp;
  TBranch        *b_sca29;
  TBranch        *b_sca30;
  TBranch        *b_sca31;
  TBranch        *b_trig0;
  TBranch        *b_trig1;
  TBranch        *b_trig2;
  TBranch        *b_trig3;
  TBranch        *b_trig4;
  TBranch        *b_trighelvalue;
  TBranch        *b_trig6;
  TBranch        *b_trighelpatt;
  TBranch        *b_ticks;
  TBranch        *b_unixtime;
  T->SetBranchAddress("singl1", &singl1, &b_singl1);
  T->SetBranchAddress("singr1", &singr1, &b_singr1);
  T->SetBranchAddress("coinc1", &coinc1, &b_coinc1);
  T->SetBranchAddress("accid1", &accid1, &b_accid1);
  T->SetBranchAddress("bcm", &bcm, &b_bcm);
  T->SetBranchAddress("ledflash", &ledflash, &b_ledflash);
  T->SetBranchAddress("sca6", &sca6, &b_sca6);
  T->SetBranchAddress("sca7", &sca7, &b_sca7);
  T->SetBranchAddress("sca8", &sca8, &b_sca8);
  T->SetBranchAddress("clockheltrig", &clockheltrig, &b_clockheltrig);
  T->SetBranchAddress("mps", &mps, &b_mps);
  T->SetBranchAddress("clockheldel", &clockheldel, &b_clockheldel);
  T->SetBranchAddress("sca12", &sca12, &b_sca12);
  T->SetBranchAddress("sca13", &sca13, &b_sca13);
  T->SetBranchAddress("clocktimer", &clocktimer, &b_clocktimer);
  T->SetBranchAddress("sca15", &sca15, &b_sca15);
  T->SetBranchAddress("singl2", &singl2, &b_singl2);
  T->SetBranchAddress("singr2", &singr2, &b_singr2);
  T->SetBranchAddress("coinc2", &coinc2, &b_coinc2);
  T->SetBranchAddress("accid2", &accid2, &b_accid2);
  T->SetBranchAddress("detsumleft", &detsumleft, &b_detsumleft);
  T->SetBranchAddress("detsumright", &detsumright, &b_detsumright);
  T->SetBranchAddress("detsumboth", &detsumboth, &b_detsumboth);
  T->SetBranchAddress("appleft", &appleft, &b_appleft);
  T->SetBranchAddress("appright", &appright, &b_appright);
  T->SetBranchAddress("bpm1h01_pxm", &bpm1h01_pxm, &b_bpm1h01_pxm);
  T->SetBranchAddress("bpm1h01_pxp", &bpm1h01_pxp, &b_bpm1h01_pxp);
  T->SetBranchAddress("bpm1h01_mxm", &bpm1h01_mxm, &b_bpm1h01_mxm);
  T->SetBranchAddress("bpm1h01_mxp", &bpm1h01_mxp, &b_bpm1h01_mxp);
  T->SetBranchAddress("sca29", &sca29, &b_sca29);
  T->SetBranchAddress("sca30", &sca30, &b_sca30);
  T->SetBranchAddress("sca31", &sca31, &b_sca31);
  T->SetBranchAddress("trig0", &trig0, &b_trig0);
  T->SetBranchAddress("trig1", &trig1, &b_trig1);
  T->SetBranchAddress("trig2", &trig2, &b_trig2);
  T->SetBranchAddress("trig3", &trig3, &b_trig3);
  T->SetBranchAddress("trig4", &trig4, &b_trig4);
  T->SetBranchAddress("trighelvalue", &trighelvalue, &b_trighelvalue);
  T->SetBranchAddress("trig6", &trig6, &b_trig6);
  T->SetBranchAddress("trighelpatt", &trighelpatt, &b_trighelpatt);
  T->SetBranchAddress("ticks", &ticks, &b_ticks);
  T->SetBranchAddress("unixtime", &unixtime, &b_unixtime);

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE STACKS - REALLY JUST A COPY OF THE LEFT TYPES APPENDED WITH "_stk"
  //EMPTY SCALERS AND TRIGGERS ARE EXCLUDED
  Int_t      singl1_stk[stksz];
  Int_t      singr1_stk[stksz];
  Int_t      coinc1_stk[stksz];
  Int_t      accid1_stk[stksz];
  Int_t      bcm_stk[stksz];
  Int_t      ledflash_stk[stksz];
  Int_t      clockheltrig_stk[stksz];
  Int_t      mps_stk[stksz];
  Int_t      clockheldel_stk[stksz];
  Int_t      clocktimer_stk[stksz];
  Int_t      singl2_stk[stksz];
  Int_t      singr2_stk[stksz];
  Int_t      coinc2_stk[stksz];
  Int_t      accid2_stk[stksz];
  Int_t      detsumleft_stk[stksz];
  Int_t      detsumright_stk[stksz];
  Int_t      detsumboth_stk[stksz];
  Int_t      appleft_stk[stksz];
  Int_t      appright_stk[stksz];
  Int_t      bpm1h01_pxm_stk[stksz];
  Int_t      bpm1h01_pxp_stk[stksz];
  Int_t      bpm1h01_mxm_stk[stksz];
  Int_t      bpm1h01_mxp_stk[stksz];
  Int_t      trig0_stk[stksz];
  Int_t      trig1_stk[stksz];
  Int_t      trig2_stk[stksz];
  Int_t      trig3_stk[stksz];
  Int_t      trig4_stk[stksz];
  Int_t      trighelvalue_stk[stksz];
  Int_t      trig6_stk[stksz];
  Int_t      trighelpatt_stk[stksz];
  Int_t      ticks_stk[stksz];
  Int_t      fileeventnumber_stk[stksz];
  Int_t      unixtime_stk[stksz];


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // INITIALIZE THE STACKS
  for(Int_t i = 0; i < stksz; i++) singl1_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) singr1_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) coinc1_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) accid1_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) bcm_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) ledflash_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) clockheltrig_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) mps_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) clockheldel_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) clocktimer_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) singl2_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) singr2_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) coinc2_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) accid2_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) detsumleft_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) detsumright_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) detsumboth_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) appleft_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) appright_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) bpm1h01_pxm_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) bpm1h01_pxp_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) bpm1h01_mxm_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) bpm1h01_mxp_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig0_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig1_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig2_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig3_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig4_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trighelvalue_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trig6_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) trighelpatt_stk[i] = -1;
  for(Int_t i = 0; i < stksz; i++) ticks_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) unixtime_stk[i] = 0;
  for(Int_t i = 0; i < stksz; i++) fileeventnumber_stk[i] = 0;

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // CYCLE TOSSING
  // WHEN YOU ADD AN ERROR -- INCREMENT 'ne', GIVE IT A NAME IN 'errname' (for printout)
  const Int_t ne = 12;
  Int_t errcnts[ne];
  for(Int_t i = 0; i < ne; i++) errcnts[i] = 0;
  const char * errname[ne][100] = {"Incorrect # of Cycles in Helicity Pattern ",
                                   "Helicity Trigger/Scaler Mismatch ",
                                   "Incorrect Helicty Pattern ",
                                   "BCM Increment Too Low ",
                                   "Singles Left Increment Issue ",
                                   "Singles Right Increment Issue ",
                                   "BCM Increment Issue ",
                                   "Coincidence Increment Issue ",
                                   "Accidentals Increment Issue ",
                                   "Negative Singles Left Increment Scaler 2 ",
                                   "Negative Singles Right Inrcement Scaler 2 ",
                                   "Clock Increment Problem "
                                   };


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // HISTOGRAMS :: KEEP NHIST UPDATED FOR PRINTING AT END.
  // WHEN YOU ADD HISTOGRAM INCREMEMNT 'nhist1' AND ADD HISTOGRAM TO ARRAY
  // FOR TH2D DO THE SAME THING
  const Int_t nhist1 = 17;
  TH1F * H[nhist1];
  Int_t i_incrbin = 10000;
  Int_t i_incrmin = 0;
  Int_t i_incrmax = 10000;
  H[ 0] = new TH1F("inc_singl_1", Form("Left Increments (Module 1) : Run %i",RUNN),  i_incrbin,  i_incrmin, i_incrmax);
  H[ 1] = new TH1F("inc_singr_1", Form("Right Increments (Module 1) : Run %i",RUNN), i_incrbin,  i_incrmin, i_incrmax);
  H[ 2] = new TH1F("inc_coinc_1", Form("Coin Increments (Module 1) : Run %i",RUNN),  i_incrbin,  i_incrmin, i_incrmax);
  H[ 3] = new TH1F("inc_accid_1", Form("Accid Increments (Module 1) : Run %i",RUNN),   i_incrbin,  i_incrmin, i_incrmax);
  H[ 4] = new TH1F("inc_current", Form("Beam Current Increments : Run %i",RUNN),  i_incrbin,  i_incrmin, i_incrmax);
  H[ 5] = new TH1F("inc_singl_2", Form("Left Increments (Module 2) : Run %i",RUNN),  i_incrbin,  i_incrmin, i_incrmax);
  H[ 6] = new TH1F("inc_singr_2", Form("Right Increments (Module 2) : Run %i",RUNN), i_incrbin,  i_incrmin, i_incrmax);
  H[ 7] = new TH1F("inc_coinc_2", Form("Coin Increments (Module 2) : Run %i",RUNN), i_incrbin,  i_incrmin, i_incrmax);
  H[ 8] = new TH1F("inc_accid_2", Form("Accid Increments (Module 2) : Run %i",RUNN), i_incrbin,  i_incrmin, i_incrmax);
  H[ 9] = new TH1F("incdiffleft", Form("Left Increments (Module 1-Module 2) : Run %i", RUNN), i_incrbin, i_incrmin, i_incrmax);
  H[10] = new TH1F("incdiffrght", Form("Right Increments (Module 1-Module 2) : Run %i", RUNN), i_incrbin, i_incrmin, i_incrmax);
  H[11] = new TH1F("incdiffcoin", Form("Coin Increments (Module 1-Module 2) : Run %i", RUNN), i_incrbin, i_incrmin, i_incrmax);
  H[12] = new TH1F("incdiffaccd", Form("Accid Increments (Module 1-Module 2) : Run %i", RUNN), i_incrbin, i_incrmin, i_incrmax);
  H[13] = new TH1F("incclktimer", Form("Clock Timer Increments : Run %i", RUNN), i_incrbin,i_incrmin,i_incrmax);
  Int_t asymbin = 500;
  Int_t asymmin =  -1;
  Int_t asymmax =   1;
  H[14] = new TH1F("asym_uncr", Form("Uncorrected Asym Dist : Run %i",RUNN), asymbin, asymmin, asymmax);
  H[15] = new TH1F("asym_corr", Form("Corrected Asym Dist : Run %i",RUNN),   asymbin, asymmin, asymmax);
  H[16] = new TH1F("qasymhist", Form("Charge Asymmetry Dist : Run %i",RUNN), asymbin, asymmin, asymmax);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //GRAPHS USED FOR PLOTTING SCALERS
  TGraph * gr_sngl1 = new TGraph();
    gr_sngl1->SetTitle( Form("Left Singles Scaler : Run %i;Entry",RUNN));
    gr_sngl1->SetMarkerStyle(6); // SCALER SUMS FILLED BY HELICITY FLIP //SCALER SUMS NOTE: ARE THESE REDUNDANT?
  TGraph * gr_sngr1 = new TGraph();
    gr_sngr1->SetTitle( Form("Right Singles Scaler : Run %i;Entry",RUNN));
    gr_sngr1->SetMarkerStyle(6);
  TGraph * gr_coin1 = new TGraph();
    gr_coin1->SetTitle( Form("Coincidence Scaler (Module 1) : Run %i;Entry",RUNN));
    gr_coin1->SetMarkerStyle(6);
  TGraph * gr_accd1 = new TGraph();
    gr_accd1->SetTitle( Form("Accidental Scaler (Module 1) : Run %i;Entry",RUNN));
    gr_accd1->SetMarkerStyle(6);
  TGraph * gr_charg = new TGraph();
    gr_charg->SetTitle( Form("BCM Scaler : Run %i;Entry",RUNN));
    gr_charg->SetMarkerStyle(6);
  TGraph * gr_clock = new TGraph();
    gr_clock->SetTitle( Form("Clock Generator : Run %i;Entry",RUNN));
    gr_clock->SetMarkerStyle(6);
  TGraph * gr_ticks = new TGraph();
    gr_ticks->SetTitle( Form("Ticks : Run %i;Entry",RUNN));
    gr_ticks->SetMarkerStyle(6);

  TGraph * gr_bcmrt = new TGraph();
    gr_bcmrt->SetTitle( Form("BCM Over Time : Run %i;Entry$",RUNN) );
    gr_bcmrt->SetMarkerStyle(6); // SCALER RATES FILLED BY HELICITY FLIP
  TGraph * gr_slrat = new TGraph();
    gr_slrat->SetTitle( Form("Left Singles Rate : Run %i;Entry$",RUNN) );
    gr_slrat->SetMarkerStyle(6);
  TGraph * gr_srrat = new TGraph();
    gr_srrat->SetTitle( Form("Right Singles Rate : Run %i;Entry$",RUNN) );
    gr_srrat->SetMarkerStyle(6);
  TGraph * gr_cnrat = new TGraph();
    gr_cnrat->SetTitle( Form("Coincidence Rate : Run %i;Entry$",RUNN) );
    gr_cnrat->SetMarkerStyle(6);
  TGraph * gr_acrat = new TGraph();
    gr_acrat->SetTitle( Form("Accidentals Rate : Run %i;Entry$",RUNN) );
    gr_acrat->SetMarkerStyle(6);
  TGraph * gr_clkrt = new TGraph();
    gr_clkrt->SetTitle( Form("Clock over Time : Run %i;Entry$",RUNN) );
    gr_clkrt->SetMarkerStyle(6);

  TGraph * gr_asymm = new TGraph();
    gr_asymm->SetTitle( Form("Corrected Asymmetry : Run %i;Entry$",RUNN) );
    gr_asymm->SetMarkerStyle(6); //FILLED BY PATTERN //FILLED BY HELICITY PATTERN
  TGraph * gr_qasym = new TGraph();
    gr_qasym->SetTitle( Form("Charge Asymmetry : Run %i;Entry$",RUNN) );
    gr_qasym->SetMarkerStyle(6);
  TGraph * gr_polar = new TGraph();
    gr_polar->SetTitle( Form("Polarizaton : Run %i;Entry$",RUNN) );
    gr_polar->SetMarkerStyle(6);

  Int_t  grcyclctr = 0;              //POINT COUNTER FOR HELICITY FLIP/CYCLE GRAPHS
  Int_t  grpattctr = 0;              //POINT COUNTER FOR HELICITY PATTERN GRAPHS
  Int_t  l1scalsum = 0;              //LEFT SCALER SUM
  Int_t  r1scalsum = 0;              //RIGHT SCALER SUM
  Int_t  c1scalsum = 0;              //COINC SCALER SUM
  Int_t  a1scalsum = 0;              //ACCID SCALER SUM
  Int_t  bcmscalsm = 0;              //BCM SCALER SUM
  Int_t  clkscalsm = 0;              //CLOCK SCALER SUM
  Int_t  tckscalsm = 0;              //TICKS SCALER SUM? MAYBE?

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //VALUES USED IN CALCULATIONS ... SHOULD PROBABLY MOVE STACKS DOWN HERE

  Int_t hcycrec   = 0;             //NUMBER OF HELICITY PATTERNS FOR WHICH AN ASYMMETRY HAS BEEN RECORDED
  Int_t numhelcyc = 0;             //KEEPS TRACK OF CURRENT HELICITY FLIP IN EACH CYCLE
  Int_t skippatt  = 3;             //NUMBER OF PREV PATTERNS TO FUTURE PATTERNS TO DISCARD AFTER ANALYSIS ERROR
  Int_t disccyc   = 2;             //NUMBER OF FUTURE PATTERNS TO DISCARD TO DISCARD AFTER AALYSIS ERROR
  Int_t goodhelpatt  = -1*skippatt;//KEEPS TRACK OF THE NUMBER OF GOOD HELICITY PATTERNS.

  Double_t helsumu[2] = { 0 , 0 }; //UNCORRECTED HELICITY SUMS FOR EACH CYCLE [H0,H1]
  Double_t helsumc[2] = { 0 , 0 }; //CORRECTED HELICITY SUMS FOR EACH CYCLE (ACCIDENTAL SUBTRACTED) [H0,H1]
  Double_t bcmsums[2] = { 0 , 0 }; //BCM SUMS FOR EACH CYCLE [H0,H1]

  Double_t expclock = 0;           //EXPECTED CLOCK TIME WINDOW
  expclock = ( 1./ freq ) * 97000 ;//100KHz CLOCK ASSUMING 3% DEADTIME, IF 240Hz DO 2% DEADTIME MAYBE(?)

  Int_t rateplotcntr = 0;


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //LOAD ROOT OUTPUT FILE
  TFile * fout = new TFile( Form("%s/molana_patterns_%i.root",gSystem->Getenv("MOLLER_ROOTFILE_DIR"),RUNN) , "RECREATE");
  const Int_t npattD = 10;
  const Int_t npattI = 4;
  Double_t temppattvaluesD[npattD];
  Int_t    temppattvaluesI[npattI];
  Char_t pattnamesD[npattD][32] = {"coinavg", // D
                                   "coinasym", // D
                                   "leftavg",  // D
                                   "leftasym", // D
                                   "rightavg", // D
                                   "rightasym",// D
                                   "accidavg", // D
                                   "accidasym",// D
                                   "bcmavg",   // D
                                   "bcmasym"}; // D
  Char_t pattnamesI[npattI][32] = {"time",     // I (Unix time maybe)
                                   "pattnumb", // I
                                   "pattsign", // I
                                   "error"};   // I  
  TTree * trPatt = new TTree( "trPatt" , Form("Pattern Tree Run %d",RUNN) );
  //FIXME: EXPAND THIS FOR LOOP IF WE CHANGE ANYTHING ELSE... OR DO IT ANYWAY.  IDK.
  for(Int_t i = 0; i < npattD; i++) trPatt->Branch( pattnamesD[i] , &temppattvaluesD[i] , Form( "%s/D",pattnamesD[i] ) );
  for(Int_t i = 0; i < npattI; i++) trPatt->Branch( pattnamesI[i] , &temppattvaluesI[i] , Form( "%s/I",pattnamesI[i] ) );
 

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // START LOOPING THROUGH THE ENTRIES OF THE ROOT NTUPLE
  const Long64_t nentries = T->GetEntriesFast();
  //cout << "Total number of scaler entries in ROOT file: " << nentries << endl;
  // FIXME: IF THIS IS ZERO THEN WE NEED TO SKIP TO END!

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t entry = 0; entry < nentries; entry++) {
    nb = T->GetEntry(entry);
    nbytes += nb;

    if(printascii){
      if(entry%25==0) output2 << std::setw(6) << " ENTRY  " << std::setw(6) << "TRGPAT  " << std::setw(6) << "TRGHEL  " << std::setw(6) << "CLKHEL  " << std::setw(6) << "  COIN  " << std::setw(6) << " ACCID  " << std::setw(6) << "   BCM  " << endl;
      if(entry%25!=0) output2 << std::setw(6) << entry << "  " << std::setw(6) << trighelpatt << "  " << std::setw(6) << trighelvalue << "  " << std::setw(6) << clockheltrig << "  " << std::setw(6) << coinc1 << "  " << std::setw(6) << accid1 << "  " << std::setw(6) << bcm << endl;
    }

    fileeventnumber_stk[entry%stksz] = entry;

    //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    //PRINT OUT THE STACKS FOR VISUAL CONFIRMATION :: NO CALCULATIONS DONE HERE
    //A "*" TO THE RIGHT OF A NUMBER IS BEGGINING OF HELICITY PATTERN
    if(printascii && trighelpatt == 0 && entry != 0){
      //PRINT EVENT NUMBER STACK
      output << "evt\%1000[";
      for(Int_t i = 0; i < stksz; i++){
        if(trighelpatt_stk[i]==0 && entry%stksz == i){
          output << "▀" << std::setw(6) << fileeventnumber_stk[i]%1000 << "*";
        }else if(trighelpatt_stk[i]==0 && entry%stksz != i){
          output << "▀" << std::setw(6) << fileeventnumber_stk[i]%1000 << " ";
        }else if(trighelpatt_stk[i]!=0 && entry%stksz == i){
          output << std::setw(6) << fileeventnumber_stk[i]%1000 << "*";
        }else{
          output << std::setw(6) << fileeventnumber_stk[i]%1000 << " ";
        }
      }
      output << "]" << endl;
      //PRINT HELICITY STACK
      output << "helstack[";
      for(Int_t i = 0; i < stksz; i++){
        if(trighelpatt_stk[i]==0){
          output << "▀" << std::setw(6) << trighelvalue_stk[i] << " ";
        }else{
          output << std::setw(6) << trighelvalue_stk[i] << " ";
        }
      }
      output << "]" << endl;
      //PRINT COINC1 STACK
      output << "coinstk1[";
      for(Int_t i = 0; i < stksz; i++){
        if(trighelpatt_stk[i]==0){
          output << "▀" << std::setw(6) << coinc1_stk[i] << " ";
        }else{
          output << std::setw(6) << coinc1_stk[i] << " ";
        }
      }
      output << "]" << endl;
      //PRINT ACCIDENTAL STACK
      output << "accdstk1[";
      for(Int_t i = 0; i < stksz; i++){
        if(trighelpatt_stk[i]==0){
          output << "▀" << std::setw(6) <<  accid1_stk[i] << " ";
        }else{
          output << std::setw(6) << accid1_stk[i] << " ";
        }
      }
      output << "]" << endl;
      //PRINT BCM STACK
      output << "bcmstack[";
      for(Int_t i = 0; i < stksz; i++){
        if(trighelpatt_stk[i]==0){
          output << "▀" << std::setw(6) <<  bcm_stk[i] << " ";
        }else{
          output << std::setw(6) << bcm_stk[i] << " ";
        }
      }
      output << "]" << endl;
    }//END OF IF(printascii)

    ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    // POPULATE THE INCREMENT STACKS
    singl1_stk[entry%stksz]          = singl1;
    singr1_stk[entry%stksz]          = singr1;
    coinc1_stk[entry%stksz]          = coinc1;
    accid1_stk[entry%stksz]          = accid1;
    bcm_stk[entry%stksz]             = bcm;
    ledflash_stk[entry%stksz]        = ledflash;
    clockheltrig_stk[entry%stksz]    = clockheltrig;
    mps_stk[entry%stksz]             = mps;
    clockheldel_stk[entry%stksz]     = clockheldel;
    clocktimer_stk[entry%stksz]      = clocktimer;
    singl2_stk[entry%stksz]          = singl2;
    singr2_stk[entry%stksz]          = singr2;
    coinc2_stk[entry%stksz]          = coinc2;
    accid2_stk[entry%stksz]          = accid2;
    detsumleft_stk[entry%stksz]      = detsumleft;
    detsumright_stk[entry%stksz]     = detsumright;
    detsumboth_stk[entry%stksz]      = detsumboth;
    appleft_stk[entry%stksz]         = appleft;
    appright_stk[entry%stksz]        = appright;
    bpm1h01_pxm_stk[entry%stksz]     = bpm1h01_pxm;
    bpm1h01_pxp_stk[entry%stksz]     = bpm1h01_pxp;
    bpm1h01_mxm_stk[entry%stksz]     = bpm1h01_mxm;
    bpm1h01_mxp_stk[entry%stksz]     = bpm1h01_mxp;
    trighelvalue_stk[entry%stksz]    = trighelvalue;
    trighelpatt_stk[entry%stksz]     = trighelpatt;
    ticks_stk[entry%stksz]           = ticks;
    unixtime_stk[entry%stksz]        = unixtime;


    ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    // INTEGRATED SUMS --- THIS AVOIDS MISCONCEPTIONS IF BEAM DIES UNTIL END OF RUN
    l1scalsum += singl1;              //LEFT SCALER SUM
    r1scalsum += singr1;              //RIGHT SCALER SUM
    c1scalsum += coinc1;              //COINC SCALER SUM
    a1scalsum += accid1;              //ACCID SCALER SUM
    bcmscalsm += bcm;                 //BCM SCALER SUM
    clkscalsm += clocktimer;          //CLOCK SCALER SUM
    gr_sngl1->SetPoint( entry, entry, (Double_t)l1scalsum );
    gr_sngr1->SetPoint( entry, entry, (Double_t)r1scalsum );
    gr_coin1->SetPoint( entry, entry, (Double_t)c1scalsum );
    gr_accd1->SetPoint( entry, entry, (Double_t)a1scalsum );
    gr_charg->SetPoint( entry, entry, (Double_t)bcmscalsm );
    gr_clock->SetPoint( entry, entry, (Double_t)clkscalsm );


    ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    // NEW PATTERN
    if( trighelpatt == 0 ){
      if(printascii) output << endl;
      goodhelpatt++;               //ASSUME IT'S GOOD HELICITY PATTERN, THIS LINE SHOULD BE ONLY HERE
      if( numhelcyc != heln ){
        goodhelpatt = -1*skippatt; //LAST PATTERN DID NOT HAVE PEOPER NUMBER OF HELICITY CYCLES/FLIPS.
        errcnts[0]++;              //ADD TO ERROR COUNTING INSUFFICIENT FLIPS IN CYCLE
      }
      numhelcyc = 0;               //IF NEW CYCLE CHANGE HELICITY FLIP VALUE TO ONE
    }
    numhelcyc++;                   //INCREASE THE HELICITY FLIP COUNTER


    ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    // OTHER CUTS / PROBLEMS
    if( (abs(singl1-singl2) > 2) ){          //DO LEFT SCALER COUNTS MATCH?
      goodhelpatt = -1*skippatt;
      errcnts[4]++;
      if(printascii) output << "[" << entry << "] ERROR Left Scaler Issue" << endl;
    }
    if( (abs(singr1-singr2) > 2) ){          //DO RIGHT SCALER COUNTS MATCH?
      goodhelpatt = -1*skippatt;
      errcnts[5]++;
      if(printascii) output << "[" << entry << "] ERROR Right Scaler Issue" << endl;
    }
    if( (abs(coinc1-coinc2) > 2) ){          //DO COINCIENCE SCALER COUNTS MATCH?
      goodhelpatt = -1*skippatt;
      errcnts[7]++;
      if(printascii) output << "[" << entry << "] ERROR Coin Scaler Issue" << endl;
    }
    if( (abs(accid1-accid2) > 2) ){          //DO ACCIDENTAL SCALER COUNTS MATCH?
      goodhelpatt = -1*skippatt;
      errcnts[8]++;
      if(printascii) output << "[" << entry << "] ERROR Accidental Scaler Issue" << endl;
    }

    if(clockheltrig > 0){                    //DO SCALER AND TRIGGER HELICITIES MATCH?
      clockheltrig /= clockheltrig;
    }
    if( (clockheltrig != trighelvalue) || (clockheltrig < 0) ){
      goodhelpatt = -1*skippatt;
      errcnts[1]++;
      if(printascii) output << "[" << entry << "] ERROR Helicty Mismatch" << endl;
    }

    if( clocktimer < (expclock*0.95) || clocktimer > (expclock*1.05) ){//IS CLOCK INCREMENT ACCEPTABLE?
      goodhelpatt = -1*skippatt;
      errcnts[11]++; //FIXME: WRONG ERROR CODE
      if(printascii) output << "[" << entry << "] ERROR Clock Discrepency" << endl;
    }
    if( bcm < 0 ){                            //NEGATIVE INCREMENT SIGNIFIES A SCALAR PROBLEM
      goodhelpatt = -1*skippatt;
      errcnts[6]++;
      if(printascii) output << "[" << entry << "] ERROR Beam Current Negative" << endl;
    }

    //HOW DO WE WANT TO DEAL WITH BCM TOO LOW PROBLEM
    if( bcm < 10 ){                           //
      goodhelpatt = -1*skippatt;
      errcnts[3]++;
      if(printascii) output << "[" << entry << "] ERROR Beam Current Low / Beam Off" << endl;
    }


    //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    //PATTERN CHECK: DO WE HAVE A GOOD PATTERN THIS SHOULD ONLY CHECK AT BEGINNING OF PATTERN
    //THIS WILL HAVE TO BE MODIFIED/UPDATED/REDONE FOR LARGER PATTERN DECISIONS.
    if( goodhelpatt > 0 ){
      Int_t goodpattindex = (entry-heln)%stksz;       //INDEX TO START LOOKING AT FOR GOOD PATTERN
      if(goodpattindex < 0) goodpattindex += stksz;   //PROTECT AGAINST RETURNING NEGATIVE REMAINDER
      Int_t pattarray[heln];
      if(printascii) output << "Helicity pattern stack indices: ";
      for(Int_t i = 0; i < heln; i++){
        Int_t fillindex = (goodpattindex+i)%stksz;    //
        if(fillindex < 0) fillindex += stksz;
        pattarray[i] = trighelvalue_stk[fillindex];
        if(printascii) output << fillindex << ",";
      }
      if(printascii){
        output << "Past helicity pattern: ";
        for(Int_t j = 0; j < heln; j++) output << pattarray[j] << ",";
        output << endl;
      }
      for(Int_t k = 1; k < heln; k++){
        //FOR 4 Pattern, (pattarray[K-1]+pattarray[K])%2 VARIES AS {1,0,1} THE SAME AS K%2 WHERE K is {1,2,3}
        //FOR 8 Pattern, (pattarray[K-1]+pattarray[K])%2 VARIES AS {1,0,1,1,1,0,1} WHICH IS {1,0,1,0,1,0,1} + {0,0,0,1,0,0,0} WHERE K is {1,2,3,4,5,6,7}
        //               WE CAN HANDLE THE EXTRA 1 AS FOLLOWS: POW( 0 , fabs(K%4) ) ... WHICH IS 0**0 == 1 FOR MULTIPLES OF 4 AND 0**(n>0) FOR ANYTHING ELSE 
        if( ((k%2)+pow(0,abs(k%4))) != ((pattarray[k-1]+pattarray[k])%2) ){
          goodhelpatt = -1*skippatt;
          errcnts[2]++;
          if(printascii) output << "[" << goodpattindex << "] UGLY HELICITY PATTERN" << endl;
          break;
        }
      }
    }


    //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    //DECLARE TEMP STACKS TO WRITE PATTERN TREE
    Int_t   temphels[heln];    //helicity sign
    Float_t tempcoin[heln];    //coinc increments
    Float_t templeft[heln];    //left increments
    Float_t temprght[heln];    //right increments
    Float_t tempaccd[heln];    //accid increments
    Float_t tempcurr[heln];    //bcm increments
    Int_t   temptime(0);       //time unix
    Int_t   tempnumb(0);       //pattern number in root file
    Int_t   tempsign(0);       //beginning sign of pattern
    Int_t   temperror(0);      //errors in cycle


    //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    //LET'S WRITE SOME ASYMMETRY DATA HERE IF WE HAVE A GOOD PATTERN
    if( goodhelpatt > 0 ){ 
      //GET THE INDEX FOR THE GOOT PATERN AND START CYCLING
      Int_t goodpattindex = (entry-heln)%stksz;     //INDEX TO START AT FOR HELICITY STACK
      if(goodpattindex < 0) goodpattindex += stksz; //WHY DOES C++ SOMETIMES RETURN NEGATIVE MODULO, OR IS % A REMAINDER HERE???
      if(printascii) output << "  --> Stack Index of Most Recent Good Pattern: " << goodpattindex << ", for event " << fileeventnumber_stk[goodpattindex] << endl;
      Bool_t b_beamokay = true;

      for(Int_t i = 0; i < heln; i++){
        Int_t helstate = trighelvalue_stk[ goodpattindex ];
        if(printascii){
          output << "    --> Index: " << std::setw(3) << goodpattindex;
          output << ", helicity: " << std::setw(2) << helstate;
        }
        //SUM UP THE INCREMENTS FOR EACH HELICITY STATE
        helsumu[ helstate ] += coinc1_stk[ goodpattindex ];
        helsumc[ helstate ] += coinc1_stk[ goodpattindex ] - accid1_stk[ goodpattindex ];
        bcmsums[ helstate ] += bcm_stk[ goodpattindex ];
        if(printascii) output << ", coin: " << std::setw(5) << coinc1_stk[goodpattindex];
        if(printascii) output << ", accd: " << std::setw(5) << accid1_stk[goodpattindex];
        if(printascii) output << ", bmcq: " << std::setw(5) << bcm_stk[goodpattindex];
        if(printascii) output << endl;


        //////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
        //RECORD THE INCREMENTS IN THE TEMPORARY STACKS USED FOR THE PATTERN TREE
        temphels[i] = helstate; //helicity sign
        tempcoin[i] = coinc1_stk[goodpattindex]; //coin
        templeft[i] = singl1_stk[goodpattindex]; //left
        temprght[i] = singr1_stk[goodpattindex]; //right
        tempaccd[i] = accid1_stk[goodpattindex]; //accid
        tempcurr[i] = bcm_stk[goodpattindex];    //bcm
        if(i==0) temptime  = unixtime;
        if(i==0) tempnumb  = fileeventnumber_stk[goodpattindex];
        if(i==0) tempsign  = helstate;
        if(i==0) temperror = 0;    

        //////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
        //TODO: HERE WE WILL FILL THE INCREMENT HISTOGRAMS AND GRAPHS
        H[ 0]->Fill( singl1_stk[goodpattindex] );
        H[ 1]->Fill( singr1_stk[goodpattindex] );
        H[ 2]->Fill( coinc1_stk[goodpattindex] );
        H[ 3]->Fill( accid1_stk[goodpattindex] );
        H[ 5]->Fill( singl2_stk[goodpattindex] );
        H[ 6]->Fill( singr2_stk[goodpattindex] );
        H[ 7]->Fill( coinc2_stk[goodpattindex] );
        H[ 8]->Fill( accid2_stk[goodpattindex] );
        H[ 4]->Fill( bcm_stk[goodpattindex] );
        H[ 9]->Fill( singl1_stk[goodpattindex]-singl2_stk[goodpattindex] );
        H[10]->Fill( singr1_stk[goodpattindex]-singr2_stk[goodpattindex] );
        H[11]->Fill( coinc1_stk[goodpattindex]-coinc2_stk[goodpattindex] );
        H[12]->Fill( accid1_stk[goodpattindex]-accid2_stk[goodpattindex] );
        H[13]->Fill( clocktimer_stk[goodpattindex] );

        //FIXME: THESE BELONG OUTSIDE THE GOODCYCLES CONDITIONAL YOU FOOL.
        //l1scalsum += singl1_stk[goodpattindex];              //LEFT SCALER SUM
        //r1scalsum += singr1_stk[goodpattindex];              //RIGHT SCALER SUM
        //c1scalsum += coinc1_stk[goodpattindex];              //COINC SCALER SUM
        //a1scalsum += accid1_stk[goodpattindex];              //ACCID SCALER SUM
        //bcmscalsm += bcm_stk[goodpattindex];                 //BCM SCALER SUM
        //clkscalsm += clocktimer_stk[goodpattindex];          //CLOCK SCALER SUM
        //gr_sngl1->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)l1scalsum );
        //gr_sngr1->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)r1scalsum );
        //gr_coin1->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)c1scalsum );
        //gr_accd1->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)a1scalsum );
        //gr_charg->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)bcmscalsm );
        //gr_clock->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)clkscalsm );

        gr_bcmrt->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)bcm_stk[goodpattindex]*(1./gate) );
        gr_slrat->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)singl1_stk[goodpattindex]*(1./gate) );
        gr_srrat->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)singr1_stk[goodpattindex]*(1./gate) );
        gr_cnrat->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)coinc1_stk[goodpattindex]*(1./gate) );
        gr_acrat->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)accid1_stk[goodpattindex]*(1./gate) );
        gr_clkrt->SetPoint( grcyclctr, fileeventnumber_stk[goodpattindex], (Double_t)clocktimer_stk[goodpattindex]*(1./gate) );

        grcyclctr++;

        goodpattindex++;
        goodpattindex = goodpattindex%(stksz);
      }//END FOR LOOP OVER HELICITY CYCLES.


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      //CALCULATE THE ASYMMETRY
      //(1) DIVIDE OUT H0 AND H1 BY THE NUMBER OF HELICITY FLIPS PER CYCLE
      Double_t dhelsumu[2] = { helsumu[0] / ((Double_t)heln / 2.), helsumu[1] / ((Double_t)heln / 2.) };
      Double_t dhelsumc[2] = { helsumc[0] / ((Double_t)heln / 2.), helsumc[1] / ((Double_t)heln / 2.) };
      for(Int_t i = 0; i < 2; i++){
        dhelsumu[i] /= (Double_t)bcmsums[i] / ((Double_t)heln / 2.);
        dhelsumc[i] /= (Double_t)bcmsums[i] / ((Double_t)heln / 2.);
      }
      //(3) TAKE THE ASYMMETRY OF THE BCM NORMALIZED SUMS (H1 - H0)/(H1 + H0)
      Double_t asymu = (dhelsumu[1] - dhelsumu[0])/(dhelsumu[1] + dhelsumu[0]);
      Double_t asymc = (dhelsumc[1] - dhelsumc[0])/(dhelsumc[1] + dhelsumc[0]);
      Double_t qasym = ( ( (Double_t)bcmsums[1]-(Double_t)bcmsums[0] ) / ( (Double_t)bcmsums[1]+(Double_t)bcmsums[0] ) );
      //(4) FILL HISTOGRAMS :)
      H[14]->Fill(asymu);
      H[15]->Fill(asymc);
      H[16]->Fill(qasym);
      //(5) FILL ASYM:ENTRY$, POL:ENTRY$, QASYM:ENTRY$ GRAPHS
      gr_asymm->SetPoint(grpattctr,fileeventnumber_stk[goodpattindex],asymc);
      gr_polar->SetPoint(grpattctr,fileeventnumber_stk[goodpattindex],asymc/(ptar*anpow));
      gr_qasym->SetPoint(grpattctr,fileeventnumber_stk[goodpattindex],qasym);
      grpattctr++;
      //PRINT OUT THE SUMS AND CALCULATED ASYMMETRIES
      if(printascii){
        output << "      --> helsumu[0]: " << std::setw(7) << helsumu[0]
               << ", helsumu[1]: "         << std::setw(7) << helsumu[1] << endl;
        output << "      --> helsumc[0]: " << std::setw(7) << helsumc[0]
               << ", helsumc[1]: "         << std::setw(7) << helsumc[1] << endl;
        output << "      --> bcmsums[0]: " << std::setw(7) << bcmsums[0]
               << ", bcmsums[1]: "         << std::setw(7) << bcmsums[1] << endl;
        output << "        --> asymu: "    << std::setw(7) << asymu      << endl;
        output << "        --> asymc: "    << std::setw(7) << asymc      << endl;
        output << endl;
      }

      //RESET THE SUMS ARRAYS FOR NEXT TIME
      for(Int_t i = 0; i < 2; i++){
        helsumu[i] = 0;
        helsumc[i] = 0;
        bcmsums[i] = 0;
      }

      //REDUCE goodhelpatt BY 1 SO WE DON'T REPEAT AGAIN UNTIL THE NEXT GOOD COMPLETED CYCLE
      goodhelpatt--;

      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      //CALCULATE ASYMS AND AVERAGE AND FILL THE PATTERNS ROOT FILE
      Float_t coin0(0.),coin1(0.),coinS(0.),left0(0.),left1(0.),leftS(0.);
      Float_t rght0(0.),rght1(0.),rghtS(0.),accd0(0.),accd1(0.),accdS(0.);
      Float_t curr0(0.),curr1(0.),currS(0.);
      for(Int_t i = 0; i < heln; i++){
        if(temphels[i] == 0){
          coin0 += tempcoin[i];
          left0 += templeft[i];
          rght0 += temprght[i];
          accd0 += tempaccd[i];
          curr0 += tempcurr[i];
        } else {
          coin1 += tempcoin[i];
          left1 += templeft[i];
          rght1 += temprght[i];
          accd1 += tempaccd[i];
          curr1 += tempcurr[i];
        }
      }

      temppattvaluesD[ 0] = (coin1 + coin0) / heln;
      temppattvaluesD[ 1] = (coin1 - coin0) / (coin1 + coin0); 
      temppattvaluesD[ 2] = (left1 + left0) / heln; 
      temppattvaluesD[ 3] = (left1 - left0) / (left1 + left0); 
      temppattvaluesD[ 4] = (rght1 + rght0) / heln; 
      temppattvaluesD[ 5] = (rght1 - rght0) / (rght1 + rght0); 
      temppattvaluesD[ 6] = (accd1 + accd0) / heln; 
      temppattvaluesD[ 7] = (accd1 - accd0) / (accd1 + accd0); 
      temppattvaluesD[ 8] = (curr1 + curr0) / heln; 
      temppattvaluesD[ 9] = (curr1 - curr0) / (curr1 + curr0); 
      temppattvaluesI[ 0] = temptime; 
      temppattvaluesI[ 1] = tempnumb; //numb
      temppattvaluesI[ 2] = tempsign; //sign
      temppattvaluesI[ 3] = 0;        //FIXME: There are no errors this way... :(

      trPatt->Fill();

    }//END ASYMMETRY CALCULATION IF(goodhelpatt > skippatt)
 
  }//END OF FOR(ENTRIES)

  fout->Write();

  cout << "molana_anaysis.C() ==> Finished analyzing increments file and wrote 'patterns' ROOT file." << endl;

  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //FITTING AND PLOTTING
  gStyle->SetOptFit(111);

  TCanvas * c_scalersums = new TCanvas("c_scalersums","c_scalersums",1500,1000);
  c_scalersums->Divide(3,2);
  c_scalersums->cd(1);
  gr_sngl1->Draw("AP");
  c_scalersums->cd(2);
  gr_sngr1->Draw("AP");
  c_scalersums->cd(3);
  gr_charg->Draw("AP");
  c_scalersums->cd(4);
  gr_coin1->Draw("AP");
  c_scalersums->cd(5);
  gr_accd1->Draw("AP");
  c_scalersums->cd(6);
  gr_clock->Draw("AP");

  TCanvas * c_increment1 = new TCanvas("c_increment1","c_increment1",1500,1000);
  Int_t sidebuff = 100;
  c_increment1->Divide(3,2);
  c_increment1->cd(1)->SetLogy();
  H[ 0]->GetXaxis()->SetRangeUser(H[ 0]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 0]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 0]->Draw();
  c_increment1->cd(2)->SetLogy();
  H[ 5]->GetXaxis()->SetRangeUser(H[ 5]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 5]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 5]->Draw();
  c_increment1->cd(3)->SetLogy();
  H[ 9]->GetXaxis()->SetRangeUser(H[ 9]->FindFirstBinAbove( 0. , 1 )-sidebuff/10,H[ 9]->FindLastBinAbove ( 0. , 1 )+sidebuff/10);
  H[ 9]->Draw();
  c_increment1->cd(4)->SetLogy();
  H[ 1]->GetXaxis()->SetRangeUser(H[ 1]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 1]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 1]->Draw();
  c_increment1->cd(5)->SetLogy();
  H[ 6]->GetXaxis()->SetRangeUser(H[ 6]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 6]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 6]->Draw();
  c_increment1->cd(6)->SetLogy();
  H[10]->GetXaxis()->SetRangeUser(H[10]->FindFirstBinAbove( 0. , 1 )-sidebuff/10,H[10]->FindLastBinAbove ( 0. , 1 )+sidebuff/10);
  H[10]->Draw();

  TCanvas * c_increment2 = new TCanvas("c_increment2","c_increment2",1500,1000);
  c_increment2->Divide(3,2);
  c_increment2->cd(1)->SetLogy();
  H[ 2]->GetXaxis()->SetRangeUser(H[ 2]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 2]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 2]->Draw();
  c_increment2->cd(2)->SetLogy();
  H[ 7]->GetXaxis()->SetRangeUser(H[ 7]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 7]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 7]->Draw();
  c_increment2->cd(3)->SetLogy();
  H[11]->GetXaxis()->SetRangeUser(H[11]->FindFirstBinAbove( 0. , 1 )-sidebuff/10,H[11]->FindLastBinAbove ( 0. , 1 )+sidebuff/10);
  H[11]->Draw();
  c_increment2->cd(4)->SetLogy();
  H[ 3]->GetXaxis()->SetRangeUser(H[ 3]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 3]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 3]->Draw();
  c_increment2->cd(5)->SetLogy();
  H[ 8]->GetXaxis()->SetRangeUser(H[ 8]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 8]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 8]->Draw();
  c_increment2->cd(6)->SetLogy();
  H[12]->GetXaxis()->SetRangeUser(H[12]->FindFirstBinAbove( 0. , 1 )-sidebuff/10,H[12]->FindLastBinAbove ( 0. , 1 )+sidebuff/10);
  H[12]->Draw();

  TCanvas * c_increment3 = new TCanvas("c_increment3","c_increment3",1500,500);
  c_increment3->Divide(2,1);
  c_increment3->cd(1)->SetLogy();
  H[ 4]->GetXaxis()->SetRangeUser(H[ 4]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 4]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 4]->Draw();
  c_increment3->cd(2)->SetLogy();
  H[13]->GetXaxis()->SetRangeUser(H[13]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[13]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[13]->Draw();

  TCanvas * c_asymhistos = new TCanvas("c_asymhistos","c_asymhistos",1500,500);
  Double_t bufffact = 0.1;
  c_asymhistos->Divide(2,1);
  c_asymhistos->cd(1);
  H[14]->Draw();
  H[14]->Fit("gaus");
  TF1 * fit5 = H[14]->GetFunction("gaus");
  fit5->SetParNames("Const","Mean","Sigma");
  H[14]->GetXaxis()->SetRangeUser( (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[14]->FindFirstBinAbove(0,1)*(1-bufffact)+(Double_t)asymmin) ,
                                   (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[14]->FindLastBinAbove (0,1)*(1+bufffact)+(Double_t)asymmin) );
  H[14]->Draw();
  c_asymhistos->cd(2);
  H[15]->Draw();
  H[15]->Fit("gaus");
  TF1 * fit6 = H[15]->GetFunction("gaus");
  fit6->SetParNames("GausConst","GausMean","GausSigma");
  H[15]->GetXaxis()->SetRangeUser( (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[15]->FindFirstBinAbove(0,1)*(1-bufffact)+(Double_t)asymmin) ,
                                   (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[15]->FindLastBinAbove (0,1)*(1+bufffact)+(Double_t)asymmin) );
  H[15]->Draw();

  TCanvas * c_grasymmtry = new TCanvas("c_grasymmtry","c_grasymmtry",1500,500);
  gr_asymm->Draw("AP");
  gr_asymm->Fit("pol0");
  TF1 * fitgrasym = gr_asymm->GetFunction("pol0");
  gr_asymm->Draw("AP");

  TCanvas * c_grpolarizn = new TCanvas("c_grpolarizn","c_grpolarizn",1500,500);
  gr_polar->Draw("AP");
  gr_polar->Fit("pol0");
  TF1 * fitgrpolr = gr_polar->GetFunction("pol0");
  gr_polar->Draw("AP");

  TCanvas * c_grcoinrate = new TCanvas("c_grcoinrate","c_grcoinrate",1500,500);
  gr_cnrat->Draw("AP");
  gr_cnrat->Fit("pol0");
  TF1 * fitgrcrat = gr_cnrat->GetFunction("pol0");
  gr_cnrat->Draw("AP");

  TCanvas * c_grleftrate = new TCanvas("c_grleftrate","c_grleftrate",1500,500);
  gr_slrat->Draw("AP");
  gr_slrat->Fit("pol0");
  TF1 * fitgrlrat = gr_slrat->GetFunction("pol0");
  gr_slrat->Draw("AP");

  TCanvas * c_grrghtrate = new TCanvas("c_grrghtrate","c_grrghtrate",1500,500);
  gr_srrat->Draw("AP");
  gr_srrat->Fit("pol0");
  TF1 * fitgrrrat = gr_srrat->GetFunction("pol0");
  gr_srrat->Draw("AP");

  TCanvas * c_graccdrate = new TCanvas("c_graccdrate","c_graccdrate",1500,500);
  gr_acrat->Draw("AP");
  gr_acrat->Fit("pol0");
  TF1 * fitgrarat = gr_acrat->GetFunction("pol0");
  gr_acrat->Draw("AP");

  TCanvas * c_grchrgasym = new TCanvas("c_grchrgasym","c_grchrgasym",1500,500);
  gr_qasym->Draw("AP");
  gr_qasym->Fit("pol0");
  TF1 * fitgrqasm = gr_qasym->GetFunction("pol0");
  gr_qasym->Draw("AP");

  TCanvas * c_grchrgrate = new TCanvas("c_grchrgrate", "c_grchrgrate", 1500,500);
  gr_bcmrt->Draw("AP");
  gr_bcmrt->Fit("pol0");
  TF1 * fitgrqrat = gr_bcmrt->GetFunction("pol0");
  gr_bcmrt->Draw("AP");

  TCanvas * c_grclockgen = new TCanvas("c_grclockgen", "c_grclockgen", 1500,500);
  gr_clkrt->Draw("AP");
  gr_clkrt->Fit("pol0");
  TF1 * fitgrclck = gr_clkrt->GetFunction("pol0");
  gr_clkrt->Draw("AP");


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // SAVE RUN DATA SUMMARY -- RUN,SINGL,SINGR,COINC,ACCID,BCM,ASYM,ASYMERR,POL,POLERR
  ofstream summary;
  std::stringstream ssof;
  ssof << "molana_prompt_stats_" << RUNN;
  //cout << "writing stats summary to file name: " << (ssof.str()).c_str() << endl;
  summary.open( (ssof.str()).c_str() );
  summary << "  RUN      LEFT     RIGHT    COIN    ACCD     BCM   CLOCK      ASYM    AERR        POL   POLERR       A_Q     A_QERR" << endl;
  Int_t    lrat = (Int_t)(fitgrlrat->GetParameter(0) );	//LEFT SINGLES RATE
  Int_t    rrat = (Int_t)(fitgrrrat->GetParameter(0) );	//RIGHT SINGLES RATE
  Int_t    crat = (Int_t)(fitgrcrat->GetParameter(0) ); //COINCIDENCE RATE
  Int_t    arat = (Int_t)(fitgrarat->GetParameter(0) ); //ACCIDENTALS RATE
  Int_t    qrat = (Int_t)(fitgrqrat->GetParameter(0) ); //CHARGE (BCM) RATE
  Int_t    clrt = (Int_t)(fitgrclck->GetParameter(0) ); //CLOCK RATE
  Double_t aavg = fitgrasym->GetParameter(0);		//ASYMMETRY MEAN FROM POL0 FIT
  Double_t aerr = fitgrasym->GetParError(0);		//ASYMMETRY MEAN ERROR FROM POL0 FIT
  Double_t pavg = fitgrpolr->GetParameter(0);		//POLARIZATION MEAN FROM POL0 FIT
  Double_t perr = fitgrpolr->GetParError(0);		//POLARIZATION MEAN ERROR FROM POL0 FIT
  Double_t qamn = fitgrqasm->GetParameter(0);		//CHARGE ASYMMETRY MEAN
  Double_t qaer = fitgrqasm->GetParError(0);		//CHARGE ASYMMETRY MEAN ERROR
  summary << std::fixed 
          << std::setw(5) << RUNN << "  "
          << std::setw(8) << lrat << "  "
          << std::setw(8) << rrat << "  "
          << std::setw(6) << crat << "  "
          << std::setw(6) << arat << "  "
          << std::setw(6) << qrat << "  "
          << std::setw(6) << clrt << "  "
          << std::setw(8) << std::setprecision(5) << aavg << "  "
          << std::setw(8) << std::setprecision(5) << aerr << "  "
          << std::setw(7) << std::setprecision(4) << pavg << "  "
          << std::setw(7) << std::setprecision(4) << perr << "  "
          << std::setw(8) << std::setprecision(5) << qamn << "  "
          << std::setw(8) << std::setprecision(5) << qaer << "  "
          << endl;
  summary.close();


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //SAVE TO PDF - 'str_savefrontpage' FIRST PAGE, 'str_saveinnerpage' FOR MIDDLE PAGES, 'str_savefinalpage' FOR LAST PAGE
  if(writetopdf){
    cout << "molana_anaysis.C() ==> Saving plots in PDF format." << endl;
    TString str_savefrontpage("");
    str_savefrontpage.Form("analysis_%i.pdf(",RUNN);
    TString str_saveinnerpage("");
    str_saveinnerpage.Form("analysis_%i.pdf",RUNN);
    TString str_savefinalpage("");
    str_savefinalpage.Form("analysis_%i.pdf)",RUNN);
    c_scalersums->SaveAs(str_savefrontpage);
    c_increment1->SaveAs(str_saveinnerpage);
    c_increment2->SaveAs(str_saveinnerpage);
    c_asymhistos->SaveAs(str_saveinnerpage);
    c_grcoinrate->SaveAs(str_saveinnerpage);
    c_grleftrate->SaveAs(str_saveinnerpage);
    c_grrghtrate->SaveAs(str_saveinnerpage);
    c_graccdrate->SaveAs(str_saveinnerpage);
    c_grchrgrate->SaveAs(str_saveinnerpage);
    c_grasymmtry->SaveAs(str_saveinnerpage);
    c_grchrgasym->SaveAs(str_saveinnerpage);
    c_grclockgen->SaveAs(str_saveinnerpage);
    c_grpolarizn->SaveAs(str_savefinalpage);
  }

  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //SAVE TO PNG -- WILL DISPLAY ON WEB IN NUMERICAL ORDER OF FIRST TWO DIGITS - SAVE ACCORDINGLY
  cout << "molana_anaysis.C() ==> Saving plots to PNG images." << endl;
  c_scalersums->SaveAs( Form( "00_analysis_%i_scalers_sum_graphs.png", RUNN) );
  c_increment1->SaveAs( Form( "01_analysis_%i_increments1_histos.png", RUNN) );
  c_increment2->SaveAs( Form( "02_analysis_%i_increments2_histos.png", RUNN) );
  c_increment3->SaveAs( Form( "03_analysis_%i_increments3_histos.png", RUNN) );
  c_grleftrate->SaveAs( Form( "04_analysis_%i_left_singles_rates.png", RUNN) );
  c_grrghtrate->SaveAs( Form( "05_analysis_%i_right_singles_rate.png", RUNN) );
  c_grcoinrate->SaveAs( Form( "06_analysis_%i_coin_rate_over_run.png", RUNN) );
  c_graccdrate->SaveAs( Form( "07_analysis_%i_accd_rate_over_run.png", RUNN) );
  c_grclockgen->SaveAs( Form( "08_analysis_%i_clock_against_time.png", RUNN) );
  c_grchrgrate->SaveAs( Form( "09_analysis_%i_bcm_rate_over_time.png", RUNN) );
  c_grchrgasym->SaveAs( Form( "10_analysis_%i_bcm_asymmetry_time.png", RUNN) );
  c_asymhistos->SaveAs( Form( "11_analysis_%i_asymm_hists_fitted.png", RUNN) );
  c_grasymmtry->SaveAs( Form( "12_analysis_%i_asymm_over_time_gr.png", RUNN) );
  c_grpolarizn->SaveAs( Form( "13_analysis_%i_pol_over_time_grph.png", RUNN) );


  if(printascii) output.close();


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // WRITE DATA STATS TO SQL DATABASE ON START ... THIS WAY RUN DATA IS INSERTED EVEN IF DATA IS BAD
  if(writetosql){
    cout << "molana_anaysis.C() ==> Writing prompt stats to " << db_host << "." << endl;
    TSQLServer * ServerEnd = TSQLServer::Connect(db_host,db_user,db_pass);
    TString queryEnd = "";
    queryEnd.Form("replace into moller_run (id_run,run_leftrate,run_rightrate,run_coinrate,run_accrate,run_bcm,run_clock,run_asym,run_asymerr,run_anpow,run_ptarg,run_pol,run_polerr,run_qasym,run_qasymerr) values (%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f)",RUNN,lrat,rrat,crat,arat,qrat,clrt,aavg,aerr,anpow,ptar,pavg,perr,qamn,qaer);
    TSQLResult * resultEnd = ServerEnd->Query(queryEnd.Data());
    ServerEnd->Close();
  }


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // PRINT ERRORS
  ofstream errorsummary;
  std::stringstream sseof;
  sseof << "errors_" << RUNN << ".txt";
  //cout << "writing error summary to file name: " << (sseof.str()).c_str() << endl;
  errorsummary.open( (sseof.str()).c_str() );
  errorsummary << "Reasons/Errors for Cycles Discarded: " << endl;
  errorsummary << "  Total root file entries: " << nentries << endl;
  for(Int_t j = 0; j <= ne; j++){
    if( j >= ne ) break;  //SAFETY STATEMENT, WAS CROSSING 'ne' FOR SOME REASON.
    errorsummary << "  " << (*errname)[j] << ": " << errcnts[j] << endl;
  }
  errorsummary.close();

}
