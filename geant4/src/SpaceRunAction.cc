#include "SpaceRunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "G4UIcommand.hh"

SpaceRunAction::SpaceRunAction(G4String n, G4int f, long s1, long s2,
                               std::string sContent, std::string mContent)
    : fileName(n), fileNumber(f), seed1(s1), seed2(s2), specContent(sContent),
      macroContent(mContent) {
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  analysisManager->SetNtupleMerging(true);
  analysisManager->SetVerboseLevel(0);

  // ---------- Histograms ----------
  // Change these ranges to match your detector/world size.
  analysisManager->CreateH2("scatter_xy", "Scatter distribution;X;Y", 200,
                            -100., 100., 200, -100., 100.);

  analysisManager->CreateH1("scatter_z", "Scatter Z distribution;Z;Counts", 200,
                            -100., 100.);

  // ---------- Ntuples ----------
  analysisManager->CreateNtuple("Scatters", "Scatters");
  analysisManager->CreateNtupleDColumn("fX");
  analysisManager->CreateNtupleDColumn("fY");
  analysisManager->CreateNtupleDColumn("fZ");
  analysisManager->CreateNtupleIColumn("event_id"); // ADD THIS
  analysisManager->FinishNtuple();

  analysisManager->CreateNtuple("Seeds", "Seeds");
  analysisManager->CreateNtupleDColumn("Seed1");
  analysisManager->CreateNtupleDColumn("Seed2");
  analysisManager->FinishNtuple();

  analysisManager->CreateNtuple("Inputs", "Inputs");
  analysisManager->CreateNtupleSColumn("macro");
  analysisManager->CreateNtupleSColumn("spec");
  analysisManager->FinishNtuple();

  G4cout << "Done " << analysisManager->GetType() << G4endl;
}

SpaceRunAction::~SpaceRunAction() {}

void SpaceRunAction::BeginOfRunAction(const G4Run *) {
  G4String generatefilename;
  auto *analysisManager = G4AnalysisManager::Instance();

  if (fileName.empty()) {
    generatefilename = "lightBall";
  } else {
    generatefilename = fileName;
  }

  if (fileNumber != -1) {
    generatefilename += "_";
    generatefilename += G4UIcommand::ConvertToString(fileNumber);
  }
  generatefilename += ".root";

  analysisManager->OpenFile(generatefilename);

  if (IsMaster()) {
    analysisManager->FillNtupleDColumn(1, 0, static_cast<G4double>(seed1));
    analysisManager->FillNtupleDColumn(1, 1, static_cast<G4double>(seed2));
    analysisManager->AddNtupleRow(1);

    analysisManager->FillNtupleSColumn(2, 0, macroContent);
    analysisManager->FillNtupleSColumn(2, 1, specContent);
    analysisManager->AddNtupleRow(2);
  }
}

void SpaceRunAction::EndOfRunAction(const G4Run *) {
  auto *analysisManager = G4AnalysisManager::Instance();

  if (IsMaster()) {
    G4cout << "------- Master -------\n"
           << "------- End of Global Run -------\n"
           << std::endl;
  } else {
    G4cout << "------- End of Local Run -------\n";
  }

  G4cout << "Writing ROOT file" << G4endl;
  analysisManager->Write();
  analysisManager->CloseFile();
}
