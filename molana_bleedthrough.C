//**************************************************************** (╯°□°）╯︵ ┻━┻
//
//  Macro currently takes three arguments
//     (1) FILE -- [Filename as string]
//     (2) FREQ -- [frequency in hertz]
//     (3) BEAM -- [beam status 0:OFF, 1:ON]
//                 If beam on  (1/true)  we can look for beam trips for bcm pedestals.
//                 If beam off (0/false) we can explicity measure bleed-through and bcm pedestal
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

void molana_bleedthrough(string FILE, Double_t FREQ, Bool_t BEAM ){

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //ANALYSIS PARAMETERS
  Bool_t   writetopdf = false;                //SAVE RESULTS ALSO IN PDF
  Bool_t   writetosql = true;                 //WRITE RESULTS TO SQL DATABASE
  Bool_t   printascii = false;                //PRINT THE STACKS TO ASCII FILE FOR VISUAL INSPECTION

  const Double_t tsettle    = 0.000090;       //90 MICROSECOND TSETTLE TIME
  const Double_t freq       = (Double_t)FREQ; //DATA COLLECTION FREQUENCY
  const Double_t gate       = 1./freq-tsettle;//ACTIVE GATE WHILE TAKING DATA
  const Double_t anpow      = 0.000;
  const Double_t ptar       = 0.000;


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
  //STRIP RUN NUMBER FROM FILE NAME
  std::string fnametrun = FILE.substr( FILE.find_last_of("\\/")+1 );
  std::string fnamecopy = fnametrun;
  fnamecopy.erase(std::remove_if(fnamecopy.begin(),fnamecopy.end(),isnonnumber),fnamecopy.end());
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
    cout << "molana_bleedthrough.C() ==> MOLANA ROOT increments file not found" << endl;
    exit(0);
  } else {
    cout << "molana_bleedthrough.C() ==> MOLANA ROOT increments file found" << endl;
  }
  TString sizecmd("");
  sizecmd = Form("wc -c %s/moller_data_%d.root | awk '{print $1}'", gSystem->Getenv("MOLLER_ROOTFILE_DIR") ,RUNN);
  TString datafilesizestring = gSystem->GetFromPipe( sizecmd );
  Int_t datafilesizeint      = datafilesizestring.Atoi();
  if(datafilesizeint < 50000){
    cout << "molana_bleedthrough.C() ==> ROOT data file extremely small. Conversion error maybe? Exiting... " << endl;
    exit(0);
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
  // CYCLE TOSSING
  // WHEN YOU ADD AN ERROR -- INCREMENT 'ne', GIVE IT A NAME IN 'errname' (for printout)
  const Int_t ne = 4;
  Int_t errcnts[ne];
  for(Int_t i = 0; i < ne; i++) errcnts[i] = 0;
  const char * errname[ne][100] = {"BCM Increment Error",
                                   "Coincidence Increment Error",
                                   "Singles Increment Error",
                                   "Accidental Increment Error"};

  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // HISTOGRAMS :: KEEP NHIST UPDATED FOR PRINTING AT END.
  // WHEN YOU ADD HISTOGRAM INCREMEMNT 'nhist1' AND ADD HISTOGRAM TO ARRAY
  // FOR TH2D DO THE SAME THING
  const Int_t nhist1 = 8;
  TH1F * H[nhist1];
  Int_t    i_incrbin = 10000;
  Int_t    i_incrmin = 0;
  Int_t    i_incrmax = 10000;
  Double_t binoffset = 0.5;
  H[ 0] = new TH1F("hBCM_beamon", Form("BCM Beam On (Coinc > 5) : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 1] = new TH1F("hCoin_beamon", Form("Coin Beam On ( bcm #epsilon [-5#sigma_{bcm},+5#sigma_{bcm}] ) : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 2] = new TH1F("hBCM_pedestal_beamtrip", Form("BCM Pedestal on Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 3] = new TH1F("hLeft_bleed_beamtrip", Form("Left Bleed on Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 4] = new TH1F("hRght_bleed_beamtrip", Form("Right Bleed on Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 5] = new TH1F("hCoin_bleed_beamtrip", Form("Coin Bleed on Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 6] = new TH1F("hAccd_bleed_beamtrip", Form("Accid Bleed on Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);
  H[ 7] = new TH1F("hClck_bleed_beamtrip", Form("Clock Increments Beam Off : Run %i",RUNN),  i_incrbin,  i_incrmin-binoffset, i_incrmax-binoffset);

  const Long64_t nentries = T->GetEntries();
  Long64_t nbytes = 0, nb = 0;

  if(BEAM){
    ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
    // WHAT IS THE MEAN BCM WHERE THERE ARE MORE THAN A FEW COINC
    Bool_t isbeamon[nentries];
    for(Long64_t entry = 0; entry < nentries; entry++) isbeamon[entry]=false;
    nbytes = 0;
    nb = 0;
    for (Long64_t entry = 0; entry < nentries; entry++) {
      nb = T->GetEntry(entry);
      nbytes += nb;
      if( (abs(coinc1-coinc2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( coinc1 > 5 ){
        H[0]->Fill(bcm);
      }
    }
    if(H[0]->GetEntries() == 0){
      if(nentries > 5000){
        cout << "molana_bleedthrough.C() ==> File appears to have no moments of beam on OR " << endl
             << "                            has significantly low coincidence rates. Was this " << endl
             << "                            supposed to be run with the beam off option? " << endl;
        exit(0);
      }
    }
    Double_t bcm_mean_beamon = H[0]->GetMean();
    Double_t bcm_stdev_beamon = H[0]->GetStdDev();
    Double_t low_bcm_threshold_double = bcm_mean_beamon - 5 * bcm_stdev_beamon;
    Int_t    low_bcm_threshold = (Int_t)(low_bcm_threshold_double);
    cout << "molana_bleedthrough.C() ==> Value of low_bcm_threshhold: " << low_bcm_threshold << endl;

    //WHAT IS THE MEAN COIN AND STDEV WHEN BEAM IS ON?
    nbytes = 0;
    nb = 0;
    for (Long64_t entry = 0; entry < nentries; entry++) {
      nb = T->GetEntry(entry);
      nbytes += nb;  
      if( (abs(coinc1-coinc2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( abs((Int_t)((Float_t)bcm-(bcm_mean_beamon))) < (Int_t)(5*bcm_stdev_beamon) ){  //BEAM ON CONDITION
        if( isbeamon[entry] == false ){
          for(Int_t i = -15; i <= 15; i++){
            if( entry+i > 0 && entry+i < nentries-1 ) isbeamon[entry+i] = true; //NOTE WHERE BEAM IS ON AND MARK IT ON 10 INCREMENTS ON EACH SIDE
          }
        }
        H[1]->Fill(coinc1);
      }
    }

    Double_t coin_mean_beamon = H[1]->GetMean();
    Double_t coin_stdev_beamon = H[1]->GetStdDev();
    Double_t low_coin_threshold_double = coin_mean_beamon - 2 * coin_stdev_beamon;
    Int_t    low_coin_threshold        = (Int_t)(low_coin_threshold_double);
    cout << "molana_bleedthrough.C() ==> Value of low_coin_threshold: " << low_coin_threshold << endl;
    if( low_coin_threshold < 0 ){
      cout << "molana_bleedthrough.C() ==> Warning, coin increments too small to use reliably!" << endl;
      exit(0);
    }

    //TODO: IF BEAM TRIP
    nbytes = 0;
    nb = 0;
    for (Long64_t entry = 0; entry < nentries; entry++) {
      nb = T->GetEntry(entry);
      nbytes += nb;

      if( (abs(coinc1-coinc2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(singl1-singl2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(singr1-singr2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(accid1-accid2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (bcm < low_bcm_threshold && coinc1 < low_coin_threshold) && (singl1 > low_coin_threshold || singr1 > low_coin_threshold) && isbeamon[entry]==false ){
        cout << ">>>   bcm: " << bcm    << endl
             << ">>> singl: " << singl1 << endl
             << ">>> singr: " << singr1 << endl
             << ">>> coinc: " << coinc1 << endl
             << ">>> accid: " << accid1 << endl << endl;
      }
      if( bcm < low_bcm_threshold && coinc1 < low_coin_threshold && isbeamon[entry]==false){
        H[ 2]->Fill(bcm);
        H[ 3]->Fill(singl1);
        H[ 4]->Fill(singr1);
        H[ 5]->Fill(coinc1);
        H[ 6]->Fill(accid1);
        H[ 7]->Fill(clocktimer);
      }
    }

    if(H[2]->GetEntries() == 0){
      if(nentries > 5000){
        cout << "molana_bleedthrough.C() ==> Data appears to have no moments of beam was off." << endl
             << "                            Beam was good but this data is not useful for bcm " << endl
             << "                            pedestal or bleedthrough. :) " << endl;
        exit(0);
      }
    }

  cout << "molana_bleedthrough.C() ==> End pedestal and bleedthrough analysis for run taken with beam ON." << endl;

  //WAS THERE A SUFFICIENT AMOUNT OF BEAM OFF DATA DURING THE RUN TO ASCERTAIN A PEDESTAL?
  if(H[2]->GetEntries() < 500){
    cout << "molana_bleedthrough.C() ==> Amount of beam off cycles was too few. Exiting..." << endl;
    exit(0);
  }

  }//END IF(BEAM)

  if(!BEAM){
    nbytes = 0;
    nb = 0;
    for (Long64_t entry = 0; entry < nentries; entry++) {
      nb = T->GetEntry(entry);
      nbytes += nb;

      if( (abs(coinc1-coinc2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(singl1-singl2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(singr1-singr2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      if( (abs(accid1-accid2) > 2) ){                    //DO COINCIENCE SCALER COUNTS MATCH?
        continue;
      }
      H[2]->Fill(bcm);
      H[3]->Fill(singl1);
      H[4]->Fill(singr1);
      H[5]->Fill(coinc1);
      H[6]->Fill(accid1);
      H[7]->Fill(clocktimer);
    }

    if(H[2]->GetEntries() == 0){
      if(nentries > 5000){
        cout << "molana_bleedthrough.C() ==> Beam was specified as off and BCM OFF hist is empty." << endl
             << "                            Was this a BEAM ON run with no trips accidentally run " << endl
             << "                            as BEAM OFF?" << endl;
        exit(0);
      }
    }  

  cout << "molana_bleedthrough.C() ==> End pedestal and bleedthrough analysis for run taken with beam off." << endl;

  }//End IF(!BEAM)

  //WAS THIS ACTUALLY A BEAM OFF RUN? IF LEFT OR RIGHT SINGLES RATES ABOVE
  if( (Int_t)(H[3]->GetMean())*(1./gate) || (Int_t)(H[3]->GetMean())*(1./gate) ){
    cout << ">>> Amount of beam off cycles was too few. Exiting..." << endl;
    exit(0);
  }

  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // PLOT THE INCREMENT DATA
  Int_t sidebuff = 25;

  TCanvas * c_beamon_calib = new TCanvas("c_beamon_calib","c_beamon_calib",1500,500);
  c_beamon_calib->Divide(2,1);
  c_beamon_calib->cd(1);
  H[0]->GetXaxis()->SetRangeUser(H[0]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[0]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[0]->Draw();
  c_beamon_calib->cd(2);
  H[1]->GetXaxis()->SetRangeUser(H[1]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[1]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[1]->Draw();

  TCanvas * c_beamon_bleed = new TCanvas("c_beamon_bleed","c_beamon_bleed",1500,1000);
  c_beamon_bleed->Divide(3,2);
  c_beamon_bleed->cd(1)->SetLogy();
  H[ 2]->GetXaxis()->SetRangeUser(H[ 2]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 2]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 2]->Draw();
  c_beamon_bleed->cd(2)->SetLogy();
  H[ 3]->GetXaxis()->SetRangeUser(H[ 3]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 3]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 3]->Draw();
  c_beamon_bleed->cd(3)->SetLogy();
  H[ 4]->GetXaxis()->SetRangeUser(H[ 4]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 4]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 4]->Draw();
  c_beamon_bleed->cd(5)->SetLogy();
  H[ 5]->GetXaxis()->SetRangeUser(H[ 5]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 5]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 5]->Draw();
  c_beamon_bleed->cd(6)->SetLogy();
  H[ 6]->GetXaxis()->SetRangeUser(H[ 6]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[ 6]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[ 6]->Draw();


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // DATA SUMMARY
  Int_t    lrat = (Int_t)((H[3]->GetMean())*(1./gate));	//LEFT SINGLES RATE
  Int_t    rrat = (Int_t)((H[4]->GetMean())*(1./gate));	//RIGHT SINGLES RATE
  Int_t    crat = (Int_t)((H[5]->GetMean())*(1./gate)); //COINCIDENCE RATE
  Int_t    arat = (Int_t)((H[6]->GetMean())*(1./gate)); //ACCIDENTALS RATE
  Int_t    qrat = (Int_t)((H[2]->GetMean())*(1./gate)); //CHARGE (BCM) RATE
  Int_t    clrt = (Int_t)((H[7]->GetMean())*(1./gate)); //CLOCK RATE
  Double_t aavg = 0.00000;                              //ASYMMETRY MEAN FROM POL0 FIT
  Double_t aerr = 0.00000;                              //ASYMMETRY MEAN ERROR FROM POL0 FIT
  Double_t pavg = 0.00000;                              //POLARIZATION MEAN FROM POL0 FIT
  Double_t perr = 0.00000;                              //POLARIZATION MEAN ERROR FROM POL0 FIT
  Double_t qamn = 0.00000;                              //CHARGE ASYMMETRY MEAN
  Double_t qaer = 0.00000;                              //CHARGE ASYMMETRY MEAN ERROR

  if(!BEAM){
    ofstream summary;
    std::stringstream ssof;
    ssof << "molana_bleed_stats_" << RUNN;
    summary.open( (ssof.str()).c_str() );
    summary << "  RUN      LEFT     RIGHT    COIN    ACCD     BCM   CLOCK      ASYM    AERR        POL   POLERR       A_Q    A_QERR" << endl;
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
  }

  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //SAVE TO PNG -- WILL DISPLAY ON WEB IN NUMERICAL ORDER OF FIRST TWO DIGITS - SAVE ACCORDINGLY
  if(BEAM) c_beamon_calib->SaveAs( Form( "30_analysis_%i_bleedthroughcalib1.png", RUNN) );
  c_beamon_bleed->SaveAs( Form( "31_analysis_%i_bleedthroughpedes2.png", RUNN) );


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // WRITE DATA STATS TO SQL -- NEED TWO CASES HERE IF BEAM and IF !BEAM  
  // IF !BEAM FILL FILL STATS THE STANDARD WAY
  if(!BEAM && writetosql){
    cout << "molana_bleedthrough.C() ==> Writing prompt stats to " << db_host << "." << endl;
    TSQLServer * ServerEnd = TSQLServer::Connect(db_host,db_user,db_pass);
    TString queryEnd1 = "";
    queryEnd1.Form("replace into moller_run (id_run,run_leftrate,run_rightrate,run_coinrate,run_accrate,run_bcm,run_clock,run_asym,run_asymerr,run_anpow,run_ptarg,run_pol,run_polerr,run_qasym,run_qasymerr) values (%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f)",RUNN,lrat,rrat,crat,arat,qrat,clrt,aavg,aerr,anpow,ptar,pavg,perr,qamn,qaer);
    TSQLResult * resultEnd1 = ServerEnd->Query(queryEnd1.Data());
    TString queryEnd2 = "";
    queryEnd2.Form("update moller_run_details set rundet_qped = %f WHERE id_rundet = %d",H[2]->GetMean(),RUNN);
    TSQLResult * resultEnd2 = ServerEnd->Query(queryEnd2.Data());
    ServerEnd->Close();
  }
  if(BEAM && writetosql){
    cout << "molana_bleedthrough.C() ==> Recording charge pedestal calc'd from beam trips during run to " << db_host << "." << endl;
    TSQLServer * ServerEnd = TSQLServer::Connect(db_host,db_user,db_pass);
    TString queryEnd = "";
    queryEnd.Form("update moller_run_details set rundet_qped = %f WHERE id_rundet = %d",H[2]->GetMean(),RUNN);
    TSQLResult * resultEnd = ServerEnd->Query(queryEnd.Data());
    ServerEnd->Close();
  }


}
