//////////
// My
#include "LaserRunAction.hh"

// namespace
using std::lock_guard;
using std::mutex;

mutex fileMutex;

LaserRunAction::LaserRunAction(LaserEventAction *eventAction, G4String n,
                               G4int f, long s1, long s2, std::string sContent,
                               std::string mContent)
    : fEventAction(eventAction) {
  fileName = n;
  fileNumber = f;
  seed1 = s1, seed2 = s2;
  specContent = sContent;
  macroContent = mContent;
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  analysisManager->SetNtupleMerging(true);
  analysisManager->SetVerboseLevel(0);

  analysisManager->CreateNtuple("Hits", "Hits");
  analysisManager->CreateNtupleDColumn("fX");
  analysisManager->CreateNtupleDColumn("fY");
  analysisManager->CreateNtupleDColumn("time");
  analysisManager->FinishNtuple();

  // Create a separate ntuple for Seeds

  analysisManager->CreateNtuple("Seeds", "Seeds");
  analysisManager->CreateNtupleDColumn("Seed1");
  analysisManager->CreateNtupleDColumn("Seed2");
  analysisManager->FinishNtuple(); // Second ntuple index: 1

  analysisManager->CreateNtuple("Inputs", "Inputs");
  analysisManager->CreateNtupleSColumn("macro");
  analysisManager->CreateNtupleSColumn("spec");
  analysisManager->FinishNtuple();

  analysisManager->CreateH2("PhotonSurfaceEmission",
                            "Image of photon re-emission", 2000, -100, 100,
                            2000, -100, 100); // hist number 0
  analysisManager->SetH2XAxisTitle(0, "X pos (mm)");
  analysisManager->SetH2YAxisTitle(0, "Y pos (mm)");
  analysisManager->SetH2ZAxisTitle(0, "Counts");

  G4cout << "Done " << analysisManager->GetType() << G4endl;
}

LaserRunAction::~LaserRunAction() {}

void LaserRunAction::BeginOfRunAction(const G4Run *aRun) {
  G4String generatefilename;
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  if (fileName == "") {
    generatefilename += "lightBall";
  } else {
    generatefilename += fileName;
  }

  if (fileNumber != -1) {
    generatefilename += "_";
    generatefilename += G4UIcommand::ConvertToString(fileNumber);
  }
  generatefilename += ".root";
  analysisManager->OpenFile(generatefilename);
  analysisManager->Reset();

  G4int tid = G4Threading::G4GetThreadId();

  if (tid == 0) {
    analysisManager->FillNtupleDColumn(1, 0, seed1); // First seed
    analysisManager->FillNtupleDColumn(1, 1, seed2); // Second seed
    analysisManager->AddNtupleRow(1);
    analysisManager->FillNtupleSColumn(2, 0, macroContent);
    analysisManager->FillNtupleSColumn(2, 1, specContent);
    analysisManager->AddNtupleRow(2);
  }
}

void LaserRunAction::EndOfRunAction(const G4Run *aRun) {
  auto analysisManager = G4AnalysisManager::Instance();
  if (IsMaster()) {
    G4cout << "-------Master-------\n"
           << "\n------- End of Global Run -------\n"
           << endl;

  } else {
    G4cout << "\n------- End of Local Run -------\n";
  }
  G4cout << "You are going to write the root file" << G4endl;

  analysisManager->Write();
  analysisManager->CloseFile();
}
