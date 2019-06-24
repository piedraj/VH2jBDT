//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  root -l TMVAReader_VH2j.C
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
float loc_mindetajl;


// Variables read from the tree
float loc0_mll;
float loc0_mjj;
float loc0_mth;
float loc0_detajj;
float loc0_Lepton_pt   [100];
float loc0_Lepton_eta  [100];
float loc0_CleanJet_eta[100];


// User defined function
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


// Init
void init_TMVAReader_VH2j(TTree* tree)
{
  tree->SetBranchAddress("mll",          &loc0_mll);
  tree->SetBranchAddress("mjj",          &loc0_mjj);
  tree->SetBranchAddress("mth",          &loc0_mth);
  tree->SetBranchAddress("detajj",       &loc0_detajj);
  tree->SetBranchAddress("Lepton_pt",    &loc0_Lepton_pt);
  tree->SetBranchAddress("Lepton_eta",   &loc0_Lepton_eta);
  tree->SetBranchAddress("CleanJet_eta", &loc0_CleanJet_eta);

  // The variables' names and their order have to agree with those in TMVAClassification_VH2j.C
  myreader->AddVariable("mll",          &loc_mll);
  myreader->AddVariable("mjj",          &loc_mjj);
  myreader->AddVariable("mth",          &loc_mth);
  myreader->AddVariable("Lepton_pt[0]", &loc_Lepton_pt0);
  myreader->AddVariable("Lepton_pt[1]", &loc_Lepton_pt1);
  myreader->AddVariable("detajj",       &loc_detajj);
  myreader->AddVariable("mindetajl(CleanJet_eta[0],CleanJet_eta[1],Lepton_eta[0],Lepton_eta[1])", &loc_mindetajl);
                
  myreader->BookMVA("BDT", "/afs/cern.ch/user/p/piedra/work/CMSSW_projects/CMSSW_9_4_9/src/PlotsConfigurations/Configurations/VH2j/Full2017/VH2jBDT/dataset/weights/TMVAClassification_VH2j_BDT.weights.xml");
}


// Main function
float TMVAReader_VH2j(int entry, int nclass)
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

    init_TMVAReader_VH2j(multidraw::currentTree);

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
  loc_mindetajl  = mindetajl(loc0_CleanJet_eta[0],loc0_CleanJet_eta[1],loc0_Lepton_eta[0],loc0_Lepton_eta[1]);
	
  float classifier = myreader->EvaluateMulticlass(nclass, "BDT");

  if (debug) std::cout << " classifier " << classifier << std::endl;

  return classifier;
}
