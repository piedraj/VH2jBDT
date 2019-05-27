//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  root -l TMVAClassification_VH2j.C\(\"BDT\"\)
//  root -l TMVAClassification_VH2j.C\(\"BDT,BDT4,BDT6,BDT12\"\)
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TObjString.h"
#include "TPluginManager.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"

#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Tools.h"


void TMVAClassification_VH2j(TString myMethodList = "") 
{
  // Load the library
  TMVA::Tools::Instance();

  // Default MVA methods to be trained + tested
  std::map<std::string,int> Use;

  Use["BDT"]   = 0;  // Uses Adaptive Boost
  Use["BDT4"]  = 1;  // Uses Adaptive Boost
  Use["BDT6"]  = 0;  // Uses Adaptive Boost
  Use["BDT12"] = 1;  // Uses Adaptive Boost

  if (myMethodList != "") {

    for (std::map<std::string,int>::iterator it=Use.begin(); it!=Use.end(); it++) it->second = 0;

    std::vector<TString> mlist = TMVA::gTools().SplitString(myMethodList, ',');

    for (UInt_t i=0; i<mlist.size(); i++) {

      std::string regMethod(mlist[i]);

      if (Use.find(regMethod) == Use.end()) {

	std::cout << " Method \"" << regMethod << "\" not known in TMVA under this name. Choose among the following:" << std::endl;

	for (std::map<std::string,int>::iterator it=Use.begin(); it!=Use.end(); it++) std::cout << it->first << " ";

	std::cout << std::endl;

	return;
      }

      Use[regMethod] = 1;
    }
  }


  // Output file
  //----------------------------------------------------------------------------
  TString workdir = "/eos/cms/store/group/phys_higgs/cmshww/amassiro/HWWNano/Fall2017_nAOD_v1_Full2017v2/MCl1loose2017v2__MCCorr2017__btagPerEvent__l2loose__l2tightOR2017";

  TString outfileName("VH2j_TMVA.root");

  TFile* outputFile = TFile::Open(outfileName, "recreate");


  // Create the factory object. The first argument is the base of the name of all the weight files
  //----------------------------------------------------------------------------
  TString factoryName("TMVAClassification_VH2j");

  TMVA::Factory* factory = new TMVA::Factory(factoryName, outputFile,
					     "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification");

  TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");

  dataloader->AddVariable("mll",          'F');
  dataloader->AddVariable("mjj",          'F');
  dataloader->AddVariable("Lepton_pt[0]", 'F');
  dataloader->AddVariable("Lepton_pt[1]", 'F');
  dataloader->AddVariable("detajj",       'F');


  // Input files
  //----------------------------------------------------------------------------
  std::vector<TFile*> InputFiles_signal;
  std::vector<TFile*> InputFiles_background;

  InputFiles_signal.clear();
  InputFiles_background.clear();

  for (UInt_t k=0; k<50; k++) {

    if (k < 20) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HZJ_HToWW_M120__part%d.root",      workdir.Data(), k)));  // Use M125 when available
    if (k < 19) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_GluGluZH_HToWW_M125__part%d.root", workdir.Data(), k)));
    if (k < 21) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HWplusJ_HToWW_M125__part%d.root",  workdir.Data(), k)));
    if (k < 29) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HWminusJ_HToWW_M125__part%d.root", workdir.Data(), k)));

    InputFiles_background.push_back(TFile::Open(Form("%s/nanoLatino_GluGluHToWWTo2L2NuPowheg_M125__part%d.root", workdir.Data(), k)));
  }


  // Apply cuts on the signal and background samples (can be different)
  //----------------------------------------------------------------------------
  TCut mycuts;
  TCut mycutb;

  mycuts = "mll > 12 \
            && Lepton_pt[0] > 25 \
            && Lepton_pt[1] > 10 \
            && (abs(Lepton_pdgId[1]) == 13 || Lepton_pt[1] > 13) \
            && nLepton >= 2 \
            && Alt$(Lepton_pt[2],0) < 10 \
            && ptll > 30 \
            && MET_pt > 20 \
            && Lepton_pdgId[0]*Lepton_pdgId[1] == -11*13 \
            && Sum$(CleanJet_pt > 30) >= 2 \
            && abs(CleanJet_eta[0]) < 2.5 \
            && abs(CleanJet_eta[1]) < 2.5 \
            && mth > 60 \
            && mth < 125 \
            && drll < 2 \
            && Sum$(CleanJet_pt > 20 && Jet_btagDeepB[CleanJet_jetIdx] > 0.1522) == 0 \
            && mjj < 200 \
            && abs(detajj) < 3.5 \
            ";

  mycutb = mycuts;

  
  // Create factory for signal and background samples
  double tmpWeight = 1.;

  for (UInt_t i=0; i<InputFiles_signal.size(); ++i) {
    TTree* tmpsTree = (TTree*)InputFiles_signal.at(i)->Get("Events");
    dataloader->AddSignalTree(tmpsTree, tmpWeight);
  }

  for (UInt_t k=0; k<InputFiles_background.size(); ++k) {
    TTree* tmpbTree = (TTree*)InputFiles_background.at(k)->Get("Events");
    dataloader->AddBackgroundTree(tmpbTree, tmpWeight);
  }

  //dataloader->SetSignalWeightExpression    ("XSWeight*GenLepMatch2l*SFweight2l*bPogSF_CMVAL*LepCut2l__ele_mva_90p_Iso2016__mu_cut_Tight80x*LepSF2l__ele_mva_90p_Iso2016__mu_cut_Tight80x*METFilter_MC");
  //dataloader->SetBackgroundWeightExpression("XSWeight*GenLepMatch2l*SFweight2l*bPogSF_CMVAL*LepCut2l__ele_mva_90p_Iso2016__mu_cut_Tight80x*LepSF2l__ele_mva_90p_Iso2016__mu_cut_Tight80x*METFilter_MC");
  dataloader->SetSignalWeightExpression    ("XSWeight");
  dataloader->SetBackgroundWeightExpression("XSWeight");


  // Tell the dataloader to use all remaining events in the trees after training for testing
  //  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random::SplitSeed=10:NormMode=None:!V");
  //  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=2404:nTrain_Background=11671:SplitMode=Block::SplitSeed=10:NormMode=EqualNumEvents");
  //  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Alternate::SplitSeed=10:NormMode=EqualNumEvents");
  //  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Random:NormMode=NumEvents:!V");
  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Random::SplitSeed=10:NormMode=EqualNumEvents");


  // Book MVA methods
  //----------------------------------------------------------------------------
  if (Use["BDT"]) factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT",
				      "!H:!V:NTrees=250:MinNodeSize=0.5%:MaxDepth=2:BoostType=AdaBoost:AdaBoostBeta=0.1:SeparationType=GiniIndex:nCuts=20");

  if (Use["BDT4"]) factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT4",
				       "!H:!V:NTrees=800:MinNodeSize=0.5%:MaxDepth=2:BoostType=AdaBoost:AdaBoostBeta=0.1:SeparationType=GiniIndex:nCuts=20");

  if (Use["BDT6"]) factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT6",
				       "!H:!V:NTrees=500:MinNodeSize=2.5%:MaxDepth=2:BoostType=AdaBoost:AdaBoostBeta=0.1:SeparationType=GiniIndex:nCuts=500");

  if (Use["BDT12"]) factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT12",
					"!H:!V:NTrees=500:MinNodeSize=0.5%:MaxDepth=2:BoostType=AdaBoost:AdaBoostBeta=0.2:SeparationType=GiniIndex:nCuts=20");


  // Now you can tell the factory to train, test, and evaluate the MVAs
  //----------------------------------------------------------------------------
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();    


  // Save the output
  //----------------------------------------------------------------------------
  outputFile->Close();

  delete factory;
  delete dataloader;
  
  // Launch the GUI for the root macros
  if (!gROOT->IsBatch()) TMVA::TMVAGui(outfileName);
}
