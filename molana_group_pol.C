#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <sstream>
#include <TString.h>

void molana_group_pol(TString runnumbers)
{
  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //LOAD ROOT FILE
  TChain trPatt("trPatt");

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

  TObjArray * tx = runnumbers.Tokenize(",");
  cout << "Runs passed: " << tx->GetEntries() << endl;
  for (Int_t i = 0; i < tx->GetEntries(); i++){
    cout << "i: " << i << endl;
    if(i == tx->GetEntries()) break;
    std::stringstream ss;
    ss << ((TObjString *)(tx->At(i)))->String();
    Int_t run = atoi( ss.str().c_str() );
    cout << "Look for: " << Form("%s/molana_patterns_%i.root",gSystem->Getenv("MOLLER_ROOTFILE_DIR"),run) << endl;
    trPatt.Add( Form("%s/molana_patterns_%i.root",gSystem->Getenv("MOLLER_ROOTFILE_DIR"),run)  );
  }

   Double_t        coinavg;
   Double_t        coinasym;
   Double_t        leftavg;
   Double_t        leftasym;
   Double_t        rightavg;
   Double_t        rightasym;
   Double_t        accidavg;
   Double_t        accidasym;
   Double_t        bcmavg;
   Double_t        bcmasym;
   Int_t           time;
   Int_t           pattnumb;
   Int_t           pattsign;
   Int_t           error;

   TBranch        *b_coinavg;
   TBranch        *b_coinasym;
   TBranch        *b_leftavg;
   TBranch        *b_leftasym;
   TBranch        *b_rightavg;
   TBranch        *b_rightasym;
   TBranch        *b_accidavg;
   TBranch        *b_accidasym;
   TBranch        *b_bcmavg;
   TBranch        *b_bcmasym;
   TBranch        *b_time;
   TBranch        *b_pattnumb;
   TBranch        *b_pattsign;
   TBranch        *b_error;

   trPatt.SetBranchAddress("coinavg", &coinavg, &b_coinavg);
   trPatt.SetBranchAddress("coinasym", &coinasym, &b_coinasym);
   trPatt.SetBranchAddress("leftavg", &leftavg, &b_leftavg);
   trPatt.SetBranchAddress("leftasym", &leftasym, &b_leftasym);
   trPatt.SetBranchAddress("rightavg", &rightavg, &b_rightavg);
   trPatt.SetBranchAddress("rightasym", &rightasym, &b_rightasym);
   trPatt.SetBranchAddress("accidavg", &accidavg, &b_accidavg);
   trPatt.SetBranchAddress("accidasym", &accidasym, &b_accidasym);
   trPatt.SetBranchAddress("bcmavg", &bcmavg, &b_bcmavg);
   trPatt.SetBranchAddress("bcmasym", &bcmasym, &b_bcmasym);
   trPatt.SetBranchAddress("time", &time, &b_time);
   trPatt.SetBranchAddress("pattnumb", &pattnumb, &b_pattnumb);
   trPatt.SetBranchAddress("pattsign", &pattsign, &b_pattsign);
   trPatt.SetBranchAddress("error", &error, &b_error);

   TGraph * tCoinAsym = new TGraph();

   //HOW TO SIZE THESE... POINTS AREN'T EXACT... TIMES CHANGE... COULD BE GAPS...  Hmmm...


   Int_t unixtimemin(2147483647);
   Int_t unixtimemax(0);
   Long64_t nentries = trPatt.GetEntries();
   cout << "Total entries: " << nentries << endl;
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      nb = trPatt.GetEntry(jentry);   nbytes += nb;

      if(time > unixtimemax) unixtimemax = time;
      if(time < unixtimemin) unixtimemin = time;

   }


   TH2F * test = new TH2F("hTest","hTest",150,unixtimemin,unixtimemax,400,-2.,2.);


   nentries = trPatt.GetEntries();
   cout << "Total entries: " << nentries << endl;
   nbytes = 0;
   nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      nb = trPatt.GetEntry(jentry);   nbytes += nb;

      test->Fill(time,coinasym);
      



   }

   Float_t min = test->GetYaxis()->GetXmin();
   Float_t max = test->GetYaxis()->GetXmax();
   test->Draw("candle2");
   test->GetYaxis()->SetRangeUser( min, max );


}
