//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  root -l VH2j_TMVAReader.C
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "TFile.h"
#include "TLorentzVector.h"
#include "TMVA/Reader.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"


namespace multidraw {extern thread_local TTree* currentTree;}

TMVA::Reader* myreader = new TMVA::Reader();

bool debug = false;

bool initialized = false;

TString name_temp = "";


// Variables fed into the BDT
float loc_mll;
float loc_mjj;
float loc_mth;
float loc_detajj;
float loc_Lepton_pt0;
float loc_Lepton_pt1;
float loc_detaljmin;


// Variables read from the tree
float loc0_mll;
float loc0_mjj;
float loc0_mth;
float loc0_detajj;
float loc0_Lepton_pt   [100];
float loc0_Lepton_eta  [100];
float loc0_CleanJet_eta[100];


// User defined function
#include "/afs/cern.ch/user/p/piedra/work/CMSSW_projects/CMSSW_10_2_15_patch2/src/PlotsConfigurations/Configurations/VH2j/Full2016_nanoAODv4/detaljmin.C"


// Init
void init_VH2j_TMVAReader(TTree* tree)
{
  tree->SetBranchAddress("mll",          &loc0_mll);
  tree->SetBranchAddress("mjj",          &loc0_mjj);
  tree->SetBranchAddress("mth",          &loc0_mth);
  tree->SetBranchAddress("detajj",       &loc0_detajj);
  tree->SetBranchAddress("Lepton_pt",    &loc0_Lepton_pt);
  tree->SetBranchAddress("Lepton_eta",   &loc0_Lepton_eta);
  tree->SetBranchAddress("CleanJet_eta", &loc0_CleanJet_eta);

  // The variables' names and their order have to agree with those in VH2j_TMVAClassification.C
  myreader->AddVariable("mll",          &loc_mll);
  myreader->AddVariable("mjj",          &loc_mjj);
  myreader->AddVariable("mth",          &loc_mth);
  myreader->AddVariable("Lepton_pt[0]", &loc_Lepton_pt0);
  myreader->AddVariable("Lepton_pt[1]", &loc_Lepton_pt1);
  myreader->AddVariable("detajj",       &loc_detajj);
  myreader->AddVariable("detaljmin(Lepton_eta[0],Lepton_eta[1],CleanJet_eta[0],CleanJet_eta[1])", &loc_detaljmin);
                
  myreader->BookMVA("BDT", "/afs/cern.ch/user/p/piedra/work/VH2jBDT/dataset/weights/VH2j_TMVAClassification_BDT.weights.xml");
}


// Main function
float VH2j_TMVAReader(int entry)
{
  if (name_temp != multidraw::currentTree->GetCurrentFile()->GetName()) {

    std::cout << " name_temp = " << name_temp << std::endl;

    name_temp = multidraw::currentTree->GetCurrentFile()->GetName();

    std::cout << " name_temp = " << name_temp << std::endl;

    initialized = false;
  }

  if (!initialized) {

    delete myreader;

    myreader = new TMVA::Reader();

    init_VH2j_TMVAReader(multidraw::currentTree);

    std::cout << " check init" << std::endl;	

    initialized = true;		
  }

  multidraw::currentTree->GetEntry(entry);

  loc_mll        = loc0_mll;
  loc_mjj        = loc0_mjj;
  loc_mth        = loc0_mth;
  loc_detajj     = loc0_detajj;
  loc_Lepton_pt0 = loc0_Lepton_pt[0];
  loc_Lepton_pt1 = loc0_Lepton_pt[1];
  loc_detaljmin  = detaljmin(loc0_Lepton_eta[0],loc0_Lepton_eta[1],loc0_CleanJet_eta[0],loc0_CleanJet_eta[1]);
	
  float classifier = myreader->EvaluateMVA("BDT");

  if (debug) std::cout << " classifier " << classifier << std::endl;

  return classifier;
}
