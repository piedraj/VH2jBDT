//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  root -l -b -q VH2j_TMVAMultiClass.C+
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <iostream>

#include "TChain.h"
#include "TFile.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAMultiClassGui.h"
#include "TMVA/Tools.h"


// User defined function
#include "/afs/cern.ch/user/p/piedra/work/CMSSW_projects/CMSSW_10_2_15_patch2/src/PlotsConfigurations/Configurations/VH2j/Full2016_nanoAODv4/detaljmin.C"


void VH2j_TMVAMultiClass()
{
  std::cout << "\n ==> Start VH2j_TMVAMultiClass\n" << std::endl;

  TMVA::Tools::Instance();

  TString workdir = "/afs/cern.ch/user/y/yiiyama/public/hwwvirtual/Summer16/l2tightOR";

  TString outfileName = "VH2j_TMVAMultiClass.root";

  TFile* outfile = TFile::Open(outfileName, "recreate");

  TMVA::Factory* factory = new TMVA::Factory("TMVAMultiClass", outfile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=multiclass");

  TMVA::DataLoader* dataloader=new TMVA::DataLoader("dataset");

  dataloader->AddVariable("mjj",                      'F');
  dataloader->AddVariable("mll",                      'F');
  dataloader->AddVariable("ptll",                     'F');
  dataloader->AddVariable("detajj",                   'F');
  dataloader->AddVariable("dphill",                   'F');
  dataloader->AddVariable("detaljmin:=detaljmin(Lepton_eta[0],Lepton_eta[1],CleanJet_eta[0],CleanJet_eta[1])", 'F');
  dataloader->AddVariable("jet1pt:=CleanJet_pt[0]",   'F');
  dataloader->AddVariable("jet2pt:=CleanJet_pt[1]",   'F');
  dataloader->AddVariable("jet1eta:=CleanJet_eta[0]", 'F');
  dataloader->AddVariable("jet2eta:=CleanJet_eta[1]", 'F');

  TChain* VH2j = new TChain("Events");
  TChain* ggH  = new TChain("Events");
  TChain* Top  = new TChain("Events");
  TChain* WW   = new TChain("Events");

  for (UInt_t k=0; k<30; k++) {
    if (k < 1) VH2j->Add(Form("%s/nanoLatino_HZJ_HToWW_M120__part%d.root",                workdir.Data(), k));
    if (k < 1) VH2j->Add(Form("%s/nanoLatino_ggZH_HToWW_M125__part%d.root",               workdir.Data(), k));
    if (k < 1) VH2j->Add(Form("%s/nanoLatino_HWplusJ_HToWW_M125__part%d.root",            workdir.Data(), k));
    if (k < 1) VH2j->Add(Form("%s/nanoLatino_HWminusJ_HToWW_M125__part%d.root",           workdir.Data(), k));
    if (k < 4) ggH ->Add(Form("%s/nanoLatino_GluGluHToWWTo2L2NuPowheg_M125__part%d.root", workdir.Data(), k));
    if (k < 1) Top ->Add(Form("%s/nanoLatino_TTTo2L2Nu__part%d.root",                     workdir.Data(), k));  // k < 68
    if (k < 1) WW  ->Add(Form("%s/nanoLatino_WWTo2L2Nu__part%d.root",                     workdir.Data(), k));  // k <  3
  }

  TCut myCuts = "(Lepton_pdgId[0]*Lepton_pdgId[1] == -11*13) && (Lepton_pt[0] > 25.) && (Lepton_pt[1] > 10.) && (abs(Lepton_pdgId[1]) == 13 || Lepton_pt[1] > 13.) && (nLepton >= 2 && Alt$(Lepton_pt[2], 0) < 10.) && (mll > 12.) && (ptll > 30.) && (PuppiMET_pt > 20.) && (Alt$(CleanJet_pt[1], 0) > 30.) && (abs(CleanJet_eta[0]) < 2.5) && (abs(CleanJet_eta[1]) < 2.5) && (mth > 60.) &&(mth < 125.) && (drll < 2.) && (mjj < 200.) && (detajj < 3.5) && (Sum$(CleanJet_pt > 20. && abs(CleanJet_eta) < 2.5 && Jet_btagDeepB[CleanJet_jetIdx] > 0.2217) == 0)";

  gROOT->cd(outfileName + ":/");

  dataloader->AddTree(VH2j, "VH2j", 1, myCuts, "Training and Testing");
  dataloader->AddTree(ggH,  "ggH",  1, myCuts, "Training and Testing");
  dataloader->AddTree(Top,  "Top",  1, myCuts, "Training and Testing");
  dataloader->AddTree(WW,   "WW",   1, myCuts, "Training and Testing");
  
  dataloader->PrepareTrainingAndTestTree("", "SplitMode=Random:NormMode=None:!V");

  factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDTG", "!H:!V:NTrees=500:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.50:nCuts=20:MaxDepth=2:SkipNormalization=1");

  factory->TrainAllMethods();     // Train MVAs using the set of training events
  factory->TestAllMethods();      // Evaluate MVAs using the set of test events
  factory->EvaluateAllMethods();  // Evaluate and compare performance of all configured MVAs

  outfile->Close();

  std::cout << "\n ==> VH2j_TMVAMultiClass is done\n" << std::endl;

  delete factory;
  delete dataloader;
}
