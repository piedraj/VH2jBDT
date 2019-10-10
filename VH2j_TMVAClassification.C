//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  root -l VH2j_TMVAClassification.C\(\"BDT\"\)
//  root -l VH2j_TMVAClassification.C\(\"BDT,BDT4,BDT6,BDT12\"\)
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TLorentzVector.h"
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


float mindetajl(float jet1_eta,
		float jet2_eta,
		float lep1_eta,
		float lep2_eta)
{
  float themin = 999;

  float detaj1l1 = fabs(jet1_eta - lep1_eta);
  float detaj1l2 = fabs(jet1_eta - lep2_eta);
  float detaj2l1 = fabs(jet2_eta - lep1_eta);
  float detaj2l2 = fabs(jet2_eta - lep2_eta);

  if (detaj1l1 < themin) themin = detaj1l1;
  if (detaj1l2 < themin) themin = detaj1l2;
  if (detaj2l1 < themin) themin = detaj2l1;
  if (detaj2l2 < themin) themin = detaj2l2;

  return themin;
}


void VH2j_TMVAClassification(TString myMethodList = "") 
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
  TString workdir = "/afs/cern.ch/user/y/yiiyama/public/hwwvirtual/Summer16/l2tightOR";

  TString outfileName("VH2j_TMVAClassification.root");

  TFile* outputFile = TFile::Open(outfileName, "recreate");


  // Create the factory object. The first argument is the base of the name of all the weight files
  //----------------------------------------------------------------------------
  TString factoryName("VH2j_TMVAClassification");

  TMVA::Factory* factory = new TMVA::Factory(factoryName, outputFile,
					     "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification");

  TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");

  dataloader->AddVariable("mll",          'F');
  dataloader->AddVariable("mjj",          'F');
  dataloader->AddVariable("mth",          'F');
  dataloader->AddVariable("Lepton_pt[0]", 'F');
  dataloader->AddVariable("Lepton_pt[1]", 'F');
  dataloader->AddVariable("detajj",       'F');
  dataloader->AddVariable("mindetajl:=mindetajl(CleanJet_eta[0],CleanJet_eta[1],Lepton_eta[0],Lepton_eta[1])", 'F');


  // Input files
  //----------------------------------------------------------------------------
  std::vector<TFile*> InputFiles_signal;
  std::vector<TFile*> InputFiles_background;

  InputFiles_signal.clear();
  InputFiles_background.clear();

  for (UInt_t k=0; k<4; k++) {

    if (k < 1) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HZJ_HToWW_M125__part%d.root",      workdir.Data(), k)));
  //if (k < 1) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_GluGluZH_HToWW_M125__part%d.root", workdir.Data(), k)));  // Not available in current workdir
    if (k < 1) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HWplusJ_HToWW_M125__part%d.root",  workdir.Data(), k)));
    if (k < 1) InputFiles_signal.push_back(TFile::Open(Form("%s/nanoLatino_HWminusJ_HToWW_M125__part%d.root", workdir.Data(), k)));

    InputFiles_background.push_back(TFile::Open(Form("%s/nanoLatino_GluGluHToWWTo2L2NuPowheg_M125__part%d.root", workdir.Data(), k)));
  }


  // Apply cuts on the signal and background samples (can be different)
  //----------------------------------------------------------------------------
  TCut mycuts;
  TCut mycutb;

  mycuts = "MET_pt > 20 \
            && Lepton_pt[0] > 25 \
            && Lepton_pt[1] > 10 \
            && Alt$(Lepton_pt[2],0) < 10 \
            && (abs(Lepton_pdgId[1]) == 13 || Lepton_pt[1] > 13) \
            && Lepton_pdgId[0]*Lepton_pdgId[1] == -11*13 \
            && mll > 12 \
            && ptll > 30 \
            && drll < 2 \
            && Sum$(CleanJet_pt > 30) >= 2 \
            && Alt$(abs(CleanJet_eta[0]),0) < 2.5 \
            && Alt$(abs(CleanJet_eta[1]),0) < 2.5 \
            && Sum$(CleanJet_pt > 20 && Jet_btagDeepB[CleanJet_jetIdx] > 0.1522) == 0 \
            && mjj < 200 \
            && abs(detajj) < 3.5 \
            && (mth > 60 && mth < 125)";

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

//dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random::SplitSeed=10:NormMode=None:!V");
//dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=2404:nTrain_Background=11671:SplitMode=Block::SplitSeed=10:NormMode=EqualNumEvents");
//dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Alternate::SplitSeed=10:NormMode=EqualNumEvents");
//dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Random:NormMode=NumEvents:!V");
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
  //----------------------------------------------------------------------------
  if (!gROOT->IsBatch()) TMVA::TMVAGui(outfileName);
}
