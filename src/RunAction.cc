#include "RunAction.hh"
#include "G4UserRunAction.hh"
#include "Run.hh"
#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4AnalysisManager.hh"
#else
#include "g4root.hh"
#endif

#include "NPParamReader.hh"

RunAction::RunAction() : fRun(nullptr)
{
}

RunAction::~RunAction()
{
}

G4Run* RunAction::GenerateRun() {
  fRun = new Run();
  return fRun;
}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
  beginRAAt = clock();

  CLHEP::HepRandom::showEngineStatus();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();

  G4String fFileName = cmdParam->getPriorityParamAsG4String("binaryFileName", "test.root");
  analysisManager->SetFileName(fFileName);
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetVerboseLevel(1);


  analysisManager->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run* aRun) {
  endRAAt = clock();
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

  if (isMaster) {
    fRun->EndOfRun();
  }



  if (isMaster) {
    cmdParam->printStats();
  }
}


