//**************************************************************** (╯°□°）╯︵ ┻━┻
//
//  Macro currently takes three arguments
//     (1) FILE -- [Number of run, currently assumes run in same file.]
//     (2) HELN -- [4:Quartet,8:Octet]
//     (3) DELAY - [helicity delay]
//
//******************************************************************************

#include<TROOT.h>
#include<TString.h>
#include<vector>

void eric_asym(Int_t RUNN, Int_t HELN, Int_t DELAY){
  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //ANALYSIS PARAMETERS
  Bool_t b_printascii = true;
  const Int_t heln  = HELN;     //QUARTET(4) OCTET(8)
  const Int_t deln  = -1*DELAY; //HELICTY SIGNAL DELAY
  const Int_t stksz = 24;       //STACK SIZE


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //PRINT OUTPUT
  ofstream output;
  output.open("ascii.out", ios::trunc);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE DATA STACKS
  Int_t helstack[stksz];       //STACK TO SAVE INDIVIDUAL HELICITIES
  Int_t coincstk[stksz];       //STACK TO SAVE INDIVIDUAL COINCIDENCE INCREMENTS
  Int_t accstack[stksz];       //STACK TO SAVE ACCIDENTAL INCREMENTS
  Int_t bcmstack[stksz];       //STACK TO SAVE BCM INCREMENTS
  Int_t coinscalstack[stksz];  //STACK TO SAVE COINCIDENCE SCALERS
  Int_t accdscalstack[stksz];  //STACK TO SAVE ACCIDENTAL SCALERS
  Int_t bcmqscalstack[stksz];  //STACK TO SAVE BCM SCALERS
  //INITIALIZE STACKS
  for(Int_t i = 0; i < stksz; i++){
    helstack[i] = -1;
    coincstk[i] = -1;
    accstack[i] = -1;
    bcmstack[i] = -1;
    coinscalstack[i] = -1;
    accdscalstack[i] = -1;
    bcmqscalstack[i] = -1;
  }
  //COUNT ENTRIES WHERE A SCALER WAS RECORDED
  Int_t scalerctr = -1;


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //LOAD ROOT FILE
  TString sfile("");
  sfile.Form("moller_data_%i.root",RUNN);
  cout << "File name to be opened: " << sfile << endl;
  TFile * fin = new TFile( sfile );
  TTree * T;
  fin->GetObject("h1",T);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE LEAF TYPES
  UShort_t        irun;
  UChar_t         idtyp;
  UChar_t         ievtyp;
  Int_t           iret;
  UChar_t         itrig[8];
  Int_t           itick;
  Int_t           nadc;
  UShort_t        iadc[36];   //[nadc]
  Int_t           ntdc;
  UShort_t        itch[4096]; //[ntdc]
  UShort_t        itim[4096]; //[ntdc]
  UChar_t         ited[4096]; //[ntdc]
  Int_t           nhcha;
  UShort_t        ihit[10];   //[nhcha]
  Int_t           nsca;
  Int_t           isca[33];   //[nsca]


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //DECLARE ROOT BRANCHES AND SET ADDRESSES
  TBranch        *b_irun;
  TBranch        *b_idtyp;
  TBranch        *b_ievtyp;
  TBranch        *b_iret;
  TBranch        *b_itrig;
  TBranch        *b_itick;
  TBranch        *b_nadc;
  TBranch        *b_iadc;
  TBranch        *b_ntdc;
  TBranch        *b_itch;
  TBranch        *b_itim;
  TBranch        *b_ited;
  TBranch        *b_nhcha;
  TBranch        *b_ihit;
  TBranch        *b_nsca;
  TBranch        *b_isca;
  T->SetBranchAddress("irun",   &irun,   &b_irun);
  T->SetBranchAddress("idtyp",  &idtyp,  &b_idtyp);
  T->SetBranchAddress("ievtyp", &ievtyp, &b_ievtyp);
  T->SetBranchAddress("iret",   &iret,   &b_iret);
  T->SetBranchAddress("itrig",  itrig,   &b_itrig);
  T->SetBranchAddress("itick",  &itick,  &b_itick);
  T->SetBranchAddress("nadc",   &nadc,   &b_nadc);
  T->SetBranchAddress("iadc",   iadc,    &b_iadc);
  T->SetBranchAddress("ntdc",   &ntdc,   &b_ntdc);
  T->SetBranchAddress("itch",   itch,    &b_itch);
  T->SetBranchAddress("itim",   itim,    &b_itim);
  T->SetBranchAddress("ited",   ited,    &b_ited);
  T->SetBranchAddress("nhcha",  &nhcha,  &b_nhcha);
  T->SetBranchAddress("ihit",   ihit,    &b_ihit);
  T->SetBranchAddress("nsca",   &nsca,   &b_nsca);
  T->SetBranchAddress("isca",   isca,    &b_isca);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // CYCLE TOSSING
  // WHEN YOU ADD AN ERROR -- INCREMENT 'ne', GIVE IT A NAME IN 'errname' (for printout)
  const Int_t ne = 3;
  Int_t errcnts[ne];
  for(Int_t i = 0; i < ne; i++) errcnts[i] = 0;
  const char * errname[ne][100] = {"Incomplete Cycle",
                                   "Hel Trig/Scaler Mismatch",
                                   "BCM Increment Too Low"
                                   };


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // HISTOGRAMS :: KEEP NHIST UPDATED FOR PRINTING AT END.
  // WHEN YOU ADD HISTOGRAM INCREMEMNT 'nhist1' AND ADD HISTOGRAM TO ARRAY
  // FOR TH2D DO THE SAME THING
  const Int_t nhist1 = 7;
  TH1F * H[nhist1];
  H[0] = new TH1F("inc_sng_l", Form("Single Left Increments - Run %i",RUNN),  10000,  0, 10000);
  H[1] = new TH1F("inc_sng_r", Form("Single Right Increments - Run %i",RUNN), 10000,  0, 10000);
  H[2] = new TH1F("inc_coinc", Form("Coincidence Increments - Run %i",RUNN),  10000,  0, 10000);
  H[3] = new TH1F("inc_accid", Form("Accidental Increments - Run %i",RUNN),   10000,  0, 10000);
  H[4] = new TH1F("inc_bcm_q", Form("Beam Charge Incrememnts - Run %i",RUNN), 10000,  0, 10000);
  Int_t asymbin = 500;
  Int_t asymmin =  -1;
  Int_t asymmax =   1;
  H[5] = new TH1F("asym_uncr", Form("Uncorrected Asym Distro - Run %i",RUNN),  500,  -1,     1);
  H[6] = new TH1F("asym_corr", Form("Corrected Asym Distro - Run %i",RUNN),    500,  -1,     1);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //GRAPHS USED FOR PLOTTING SCALERS
  TGraph * gr_singl = new TGraph();
   gr_singl->SetTitle("Left Singles Scaler;Entry$");
   gr_singl->SetMarkerStyle(6);
  TGraph * gr_singr = new TGraph();
   gr_singr->SetTitle("Right Singles Scaler;Entry$");
   gr_singr->SetMarkerStyle(6);
  TGraph * gr_coinc = new TGraph();
   gr_coinc->SetTitle("Coincidence Scaler;Entry$");
   gr_coinc->SetMarkerStyle(6);
  TGraph * gr_accid = new TGraph();
   gr_accid->SetTitle("Accidental Scaler;Entry$");
   gr_accid->SetMarkerStyle(6);
  TGraph * gr_charg = new TGraph();
   gr_charg->SetTitle("BCM Scaler;Entry$");
   gr_charg->SetMarkerStyle(6);
  TGraph * gr_asymm = new TGraph();
   gr_asymm->SetTitle("Corrected Asymmetry;Helicity Cycle Beginning at Entry$");
   gr_asymm->SetMarkerStyle(7);


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //VALUES USED IN CALCULATIONS ... SHOULD PROBABLY MOVE STACKS DOWN HERE
  Int_t prevLeft  = 0;             //USED TO CALCULATE SINGLE LEFT INCREMENTS
  Int_t prevRght  = 0;             //USED TO CALCULATE SINGLE RIGHT INCREMENTS
  Int_t currLeft  = 0;             //USED TO CALCULATE SINGLE LEFT INCREMENTS
  Int_t currRght  = 0;             //USED TO CALCULATE SINGLE RIGHT INCREMENTS
  Int_t currCoin  = 0;             //USED TO COINCIDENCE INCREMENTS
  Int_t prevCoin  = 0;             //USED TO COINCIDENCE INCREMENTS
  Int_t prevAccd  = 0;             //USED TO ACCIDENTAL INCREMENTS
  Int_t currAccd  = 0;             //USED TO ACCIDENTAL INCREMENTS
  Int_t currbcm   = 0;             //USED TO CHARGE INCREMENTS
  Int_t prevbcm   = 0;             //USED TO CHARGE INCREMENTS

  Int_t currthel  = -1;            //CURRENT HELICITY FROM TRIGGER
  Int_t prevthel  = -1;            //PREVIOUS HELICITY FROM TRIGGER
  Int_t currshel  = -1;            //CURRENT HELICITY FROM SCALERS
  Int_t previsca9 = 0;             //KEEPS TRACK OF ISCA[9] SO THAT WE CAN GET HELICITY FROM SCALER DATA

  Int_t currcnt   = 0;             //CURRENT COIN SCALER
  Int_t prevcnt   = 0;             //PREVIOUS COIN SCALER
  Int_t curracc   = 0;             //CURRENT ACCIDENTAL SCALER
  Int_t prevacc   = 0;             //PREVIOUS ACCIDENTAL SCALER

  Int_t hcycrec   = 0;             //NUMBER OF HELICITY CYCLES FOR WHICH AN ASYMMETRY HAS BEEN RECORDED
  Int_t helflip   = -1;            //KEEPS TRACK OF CURRENT HELICITY FLIP IN EACH CYCLE
  Int_t skipcyc   =  3;            //NUMBER OF PREV CYCLES TO FUTURE CYCLES TO DISCARD AFTER ANALYSIS ERROR
  Int_t disccyc   =  2;            //NUMBER OF FUTURE CYCLES TO DISCARD TO DISCARD AFTER ANALYSIS ERROR
  Int_t gdhelcyc  = -1 * skipcyc;  //KEEPS TRACK OF GOOD HELICITY CYCLES SINCE LAST BAD.

  Double_t helsumu[2] = { 0 , 0 }; //UNCORRECTED HELICITY SUMS FOR EACH CYCLE [H0,H1]
  Double_t helsumc[2] = { 0 , 0 }; //CORRECTED HELICITY SUMS FOR EACH CYCLE (ACCIDENTAL SUBTRACTED) [H0,H1]
  Double_t bcmsums[2] = { 0 , 0 }; //BCM SUMS FOR EACH CYCLE [H0,H1]


  ///////////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // START LOOPING THROUGH THE ENTRIES OF THE ROOT NTUPLE
  const Long64_t nentries = T->GetEntriesFast();
  cout << "Total number of scaler and ADC entries in ROOT file: " << nentries << endl;
  // FIXME: IF THIS IS ZERO THEN WE NEED TO SKIP TO END!

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    nb = T->GetEntry(jentry);
    nbytes += nb;

    if( jentry%1000==0 ){ printf("Reading entry #%lli\r",jentry); fflush(stdout);}

    if(nsca > 0){
      scalerctr++;                 //INCREASE SCALER COUNTER
      helflip++;                   //INCREASE HELICITY FLIP NUMBER

       if( itrig[7] == 0 ){
        gdhelcyc++;                //INCREASE GOOD HELICITY CYCLE NUMBER
        if( helflip!=heln ){
          gdhelcycle = -1*skipcyc; //LAST CYCLE DID NOT HAVE PEOPER NUMBER OF HELICITY FLIPS
          errcnts[0]++;            //ADD TO ERROR COUNTING INSUFFICIENT FLIPS IN CYCLE
          output << "INSUFFICIENT FLIPS IN CYCLE!!! HELFLIP: " << helflip << " & HELN: " << heln << endl;
        }
        helflip  = 0;              //IF NEW CYCLE CHANGE HELICITY FLIP VALUE TO ZERO
      }


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      //LET'S WRITE SOME ASYMMETRY DATA HERE IF WE HAVE GOOD CYCLES.
      //IF(GDHELCYC > SKIPCYC) WE'VE SKIPPED THE SPECIFIED NUMBER OF CYCLES AFTER AN ANALYZING ERROR
      //FIXME: AM I SKIPPING CYCLES RIGHT?
      if( gdhelcyc > skipcyc ){
        if(b_printascii) output << endl << "Recording data from good helicity cycle! :) " << endl;
        //STACK INDEX TO CALCULATE ASYMMETRY SHOULD BE (LAST WRITTEN HELICITY - CYCLE SIZE + 1)
        Int_t helindex = ((scalerctr-1)+deln)%stksz; //INDEX FOR HELICITY STACK
        if(helindex < 0) helindex += stksz;          //WHY DOES c++ RETURN A NEGATIVE MODULO, OR IS % A REMAINDER???
        if(b_printascii) output << "Last scaler's helicity index: " << helindex << endl;
        Int_t stackindex = helindex - heln + 1;
        if(stackindex < 0) stackindex += stksz;
        if(b_printascii) output << "  --> Starting Stack Index: " << stackindex << endl;
        for(Int_t i = 0; i < heln; i++){
          //Check for proper pattern
          Bool_t helpattok = false;
          Int_t helstate = helstack[ stackindex ];
          if(b_printascii){
            output << "    --> Index: " << std::setw(3) << stackindex;
            output << ", helicity: " << std::setw(2) << helstate;
          }
          //SUM UP THE INCREMENTS FOR EACH HELICITY STATE
          helsumu[ helstate ] += coincstk[ stackindex ];
          helsumc[ helstate ] += coincstk[ stackindex ] - accstack[ stackindex ];
          bcmsums[ helstate ] += bcmstack[ stackindex ];
          if(b_printascii) output << ", coin: " << std::setw(5) << coincstk[stackindex];
          if(b_printascii) output << ", accd: " << std::setw(5) << accstack[stackindex];
          if(b_printascii) output << ", bmcq: " << std::setw(5) << bcmstack[stackindex];
          if(b_printascii) output << endl;
          //INCREMENT THE STACK INDEX AND ROLL OVER THE INDEX IF NEEDED
          stackindex++;
          stackindex = stackindex%(stksz);
        }

        //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
        //CALCULATE THE ASYMMETRY
        //(1) DIVIDE OUT H0 AND H1 BY THE NUMBER OF HELICITY FLIPS PER CYCLE
        Double_t dhelsumu[2] = { helsumu[0] / ((Double_t)heln / 2.),
                                 helsumu[1] / ((Double_t)heln / 2.) };
        Double_t dhelsumc[2] = { helsumc[0] / ((Double_t)heln / 2.),
                                 helsumc[1] / ((Double_t)heln / 2.) };
        for(Int_t i = 0; i < 2; i++){
          dhelsumu[i] /= (Double_t)bcmsums[i] / ((Double_t)heln / 2.);
          dhelsumc[i] /= (Double_t)bcmsums[i] / ((Double_t)heln / 2.);
        }
        //(3) TAKE THE ASYMMETRY OF THE BCM NORMALIZED SUMS (H0 - H1)/(H0 + H1)
        Double_t asymu = (dhelsumu[0] - dhelsumu[1])/(dhelsumu[0] + dhelsumu[1]);
        Double_t asymc = (dhelsumc[0] - dhelsumc[1])/(dhelsumc[0] + dhelsumc[1]);
        //(4) FILL HISTOGRAMS :)
        H[5]->Fill(asymu);
        H[6]->Fill(asymc);
        hcycrec++;
        //(5) FILL ASYM:ENTRY$ GRAPH
        gr_asymm->SetPoint(hcycrec,jentry-skipcyc*HELN-HELN,asymc);

        //PRINT OUT THE SUMS AND CALCULATED ASYMMETRIES
        if(b_printascii){
          output << "      --> helsumu[0]: " << std::setw(7)  << helsumu[0]
                 << ", helsumu[1]: " << std::setw(7) << helsumu[1] << endl;
          output << "      --> helsumc[0]: " << std::setw(7) << helsumc[0]
                 << ", helsumc[1]: " << std::setw(7) << helsumc[1] << endl;
          output << "      --> bcmsums[0]: " << std::setw(7) << bcmsums[0]
                 << ", bcmsums[1]: " << std::setw(7) << bcmsums[1] << endl;
          output << "        --> asymu: " << std::setw(7) << asymu << endl;
          output << "        --> asymc: " << std::setw(7) << asymc << endl;
        }
        //RESET THE SUMS ARRAYS FOR NEXT TIME
        for(Int_t i = 0; i < 2; i++){
          helsumu[i] = 0;
          helsumc[i] = 0;
          bcmsums[i] = 0;
        }

	//REDUCE GDHELCYC BY 1 SO WE DON'T REPEAT AGAIN UNTIL THE NEXT COMPLETED CYCLE
        gdhelcyc--;
      }//END ASYMMETRY CALCULATION IF(GDHELCYC > SKIPCYC)


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      //PRINT STUFF IS THIS IS THE BEGINNING OF A NEW HELICITY CYCLE.
      if( b_printascii ){
        if( itrig[7] == 0 ) output << endl << "===== NEW HELICITY CYCLE ====="
                                   << " (scaler_counter: " << scalerctr
                                   << ", mod(scaler_counter):" << scalerctr%(stksz)
                                   << ", jentry: " << jentry
                                   << ", delayed helicity: " << (Int_t)itrig[5]
                                   << " )" << endl;
      }


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      // CALCULATE THE INCREMENTS AND HELICITY OF THE CURRENT ENTRY$
      prevbcm  = currbcm;
      currbcm  = isca[4];
      gr_charg->SetPoint(scalerctr+1,scalerctr,currbcm);
      Int_t beaminc = currbcm - prevbcm;      //CALCULATE BEAM CHARGE INCREMENTS
      if(beaminc > 10){
        H[4]->Fill(beaminc);
      } else {
        beaminc    =  0;                      //NO BEAM; RESET GOOD CYCLE TRACKER TO SKIPCYCLES VALUE
        gdhelcycle = -1*skipcyc;              //RESET GOOD CYCLES SKIP NEXT FEW
        errcnts[2]++;                         //RECORD INCIDENT OF BCM COUNT TOO LOW ERROR
      }

      prevLeft = currLeft;                    //CALCULATE LEFT SINGLES INCREMENTS
      currLeft = isca[0];
      gr_singl->SetPoint(scalerctr+1,scalerctr,currLeft);
      Int_t leftinc = currLeft - prevLeft;
      if(beaminc != 0) H[0]->Fill(leftinc);

      prevRght = currRght;    	       	       //CALCULATE RIGHT SINGLES INCREMENTS
      currRght = isca[1];
      gr_singr->SetPoint(scalerctr+1,scalerctr,currRght);
      Int_t rightinc = currRght - prevRght;
      if(beaminc != 0) H[1]->Fill(rightinc);

      prevCoin = currCoin;    	       	       //CALCULATE COINCIDENCE INCREMENTS
      currCoin = isca[2];
      gr_coinc->SetPoint(scalerctr+1,scalerctr,currCoin);
      Int_t coininc = currCoin - prevCoin;
      if(beaminc != 0) H[2]->Fill(coininc);

      prevAccd = currAccd;    	       	       //CALCULATE ACCIDNENTAL INCREMENTS
      currAccd = isca[3];
      gr_accid->SetPoint(scalerctr+1,scalerctr,currAccd);
      Int_t accdinc = currAccd - prevAccd;
      if(beaminc != 0) H[3]->Fill(accdinc);

      currshel = isca[9] - previsca9;          //CALCULATE HELICITY FROM SCALERS COMPARE TO ITRIG[5]
      if(currshel > 0) currshel /= currshel;
      if(currshel != (Int_t)itrig[5] && jentry != 0){   //WHAT HAPPENS IF SCALER CALC'D HEL DOESN'T MATCH TRIGGER?
        if(b_printascii){
          output << "HELICITY MISMATCH! currshel: "
                 << currshel << ", currthel: "
                 << (Int_t)itrig[5] << endl;
        }
        gdhelcyc = -1*skipcyc;                 //RESET GOOD CYCLES AND SKIP NEXT FEW
        errcnts[1]++;                          //ERROR TRIGGER/SCALER HELICITY MISMATCH
      }
      previsca9 = isca[9];                     //SAVE THE PREVIOUS ISCA[9] TO GET NEXT HELICITY FROM SCALER


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      // POPULATE THE HELICITY, INCREMENT AND SCALER STACKS
      Int_t helindex = (scalerctr+deln)%stksz; //INDEX FOR HELICITY STACK
      if(helindex < 0) helindex += stksz;      //WHY DOES c++ RETURN A NEGATIVE MODULO, OR IS % A REMAINDER???
      Int_t incindex = (scalerctr)%stksz;      //INDEX FOR INCREMENT AND SCALER STACKS
      if(b_printascii) output << "helicityStackIndex: " << helindex << endl;
      if(b_printascii) output << "incremntStackIndex: " << incindex << endl;
      helstack[ helindex ] = currshel;         //FILL HELICITY STACK
      coincstk[ incindex ] = coininc;          //FILL COINCIDENCE INCREMENT STACK
      accstack[ incindex ] = accdinc;          //FILL ACCIDENTAL INCREMENT STACK
      bcmstack[ incindex ] = beaminc;          //FILL CHARGE INCREMENT STACK
      coinscalstack[ incindex ] = currcnt;     //FILL COINCIDENCE SCALER STACK
      accdscalstack[ incindex ] = curracc;     //FILL ACCIDENTAL SCALER STACK
      bcmqscalstack[ incindex ] = currbcm;     //FILL CHARGE SCALER STACK


      //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
      //PRINT OUT THE STACKS FOR VISUAL CONFIRMATION :: NO CALCULATIONS DONE HERE
      //A "*" TO THE RIGHT OF A NUMBER IS THE MOST RECENTLY WRITTEN NUMBER
      if(b_printascii){
        output << "(" << helflip << ") helstack[";
        for(Int_t i = 0; i < stksz; i++){
          if(helindex==i){
            output << std::setw(6) << helstack[i] << "*";
          }else{
            output << std::setw(6) << helstack[i] << " ";
          }
        }
        output << "]" << endl;
        //PRINT COINC STACK
        output << "(" << helflip << ") coincstk[";
        for(Int_t i = 0; i < stksz; i++){
          if((scalerctr)%(stksz)==i){
            output << std::setw(6) << coincstk[i] << "*";
          }else{
            output << std::setw(6) << coincstk[i] << " ";
          }
        }
        output << "]" << endl;
        //PRINT ACCIDENTAL STACK
        output << "(" << helflip << ") accstack[";
        for(Int_t i = 0; i < stksz; i++){
          if((scalerctr)%(stksz)==i){
            output << std::setw(6) << accstack[i] << "*";
          }else{
            output << std::setw(6) << accstack[i] << " ";
          }
        }
        output << "]" << endl;
        //PRINT BCM STACK
        output << "(" << helflip << ") bcmstack[";
        for(Int_t i = 0; i < stksz; i++){
          if((scalerctr)%(stksz)==i){
            output << std::setw(6) << bcmstack[i] << "*";
          }else{
            output << std::setw(6) << bcmstack[i] << " ";
          }
        }
        output << "]" << endl;
      }//END OF IF(B_PRINTASCII)

    }//END OF IF(NSCA>0)

  }//END OF FOR(ENTRIES)

  cout << "Finished cycling through ROOT file..." << endl;


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //FITTING AND PLOTTING
  gStyle->SetOptFit(111);


  TCanvas * cScalers = new TCanvas("cScalers","cScalers",1200,800);
  cScalers->Divide(3,2);
  cScalers->cd(1);
  gr_singl->Draw("AP");
  cScalers->cd(2);
  gr_singr->Draw("AP");
  cScalers->cd(3);
  gr_charg->Draw("AP");
  cScalers->cd(4);
  gr_coinc->Draw("AP");
  cScalers->cd(5);
  gr_accid->Draw("AP");

  TCanvas * cIncrements = new TCanvas("cIncrements","cIncrements",1200,800);
  Int_t sidebuff = 100;
  cIncrements->Divide(3,2);
  cIncrements->cd(1);
  H[0]->GetXaxis()->SetRangeUser(H[0]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[0]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[0]->Draw();
  //
  cIncrements->cd(2);
  H[1]->GetXaxis()->SetRangeUser(H[1]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[1]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[1]->Draw();
  //
  cIncrements->cd(3);
  H[4]->GetXaxis()->SetRangeUser(H[4]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[4]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[4]->Draw();
  //
  cIncrements->cd(4);
  H[2]->GetXaxis()->SetRangeUser(H[2]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[2]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[2]->Draw();
  //
  cIncrements->cd(5);
  H[3]->GetXaxis()->SetRangeUser(H[3]->FindFirstBinAbove( 0. , 1 )-sidebuff,H[3]->FindLastBinAbove ( 0. , 1 )+sidebuff);
  H[3]->Draw();


  TCanvas * cAsymmetries = new TCanvas("cAsymmetries","cAsymmetries",1200,400);
  Double_t bufffact = 0.1;
  cAsymmetries->Divide(2,1);
  cAsymmetries->cd(1);
  H[5]->Draw();
  H[5]->Fit("gaus");
  TF1 * fit5 = H[5]->GetFunction("gaus");
  fit5->SetParNames("Const","Mean","Sigma");
  H[5]->GetXaxis()->SetRangeUser( (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[5]->FindFirstBinAbove(0,1)*(1-bufffact)+(Double_t)asymmin) ,
                                  (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[5]->FindLastBinAbove (0,1)*(1+bufffact)+(Double_t)asymmin)
                                );
  H[5]->Draw();
  //
  cAsymmetries->cd(2);
  H[6]->Draw();
  H[6]->Fit("gaus");
  TF1 * fit6 = H[6]->GetFunction("gaus");
  fit6->SetParNames("GausConst","GausMean","GausSigma");
  H[6]->GetXaxis()->SetRangeUser( (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[6]->FindFirstBinAbove(0,1)*(1-bufffact)+(Double_t)asymmin) ,
                                  (((Double_t)asymmax-(Double_t)asymmin)/(Double_t)asymbin*(Double_t)H[6]->FindLastBinAbove (0,1)*(1+bufffact)+(Double_t)asymmin)
                                );
  H[6]->Draw();


  TCanvas * cGrAsymmByEntry = new TCanvas("cGrAsymmByEntry","cGrAsymmByEntry",1200,400);
  gr_asymm->Draw("AP");
  gr_asymm->Fit("pol0");
  TF1 * fitgrasym = gr_asymm->GetFunction("pol0");
  fitgrasym->SetParNames("Mean");
  gr_asymm->Draw("AP");


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  //SAVE TO PDF - 'sSaveFirstPage' FIRST PAGE, 'sSaveMiddlePage' FOR MIDDLE PAGES, 'sSaveLastPage' FOR LAST PAGE
  TString sSaveFirstPage("");
  sSaveFirstPage.Form("analysis_%i.pdf(",RUNN);
  TString sSaveMiddlePage("");
  sSaveMiddlePage.Form("analysis_%i.pdf",RUNN);
  TString sSaveLastPage("");
  sSaveLastPage.Form("analysis_%i.pdf)",RUNN);

  cScalers->SaveAs(sSaveFirstPage);
  cIncrements->SaveAs(sSaveMiddlePage);
  cAsymmetries->SaveAs(sSaveMiddlePage);
  cGrAsymmByEntry->SaveAs(sSaveLastPage);


  //////////////////////////////////////////////////////////  (╯°□°）╯︵ ┻━┻
  // PRINT ERRORS
  cout << "Reasons/Errors for Cycles Discarded: " << endl;
  for(Int_t i = 0; i <= ne; i++) cout << "  " << (*errname)[i] << ": " << errcnts[i] << endl;




  if(b_printascii) output.close();
}
