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

  createAnalysisData();

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

void RunAction::createAnalysisData() {
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();
  analysisManager->CreateH1("primaSpcOnWorldBoundary", "Spectrum of primaries at world boundary", 6000, 2400 * MeV, 2700 * MeV);

  G4int tubeSmallUnitZCount = cmdParam->getPriorityParamAsG4Int("tubeSmallUnitZCount", "350");
  analysisManager->CreateH1("depFull", "Deposit full, MeV", tubeSmallUnitZCount, 0, tubeSmallUnitZCount - 1);


  G4int qZ = cmdParam->getPriorityParamAsG4Int("tubeSmallUnitZCount", "1000");

  G4int topTarget4Framgents = cmdParam->getPriorityParamAsG4Int("topTarget4Fragments", "9");

  for (int i = 1; i < topTarget4Framgents; i++) {
    std::stringstream ss1, ss2;
    ss1 << "a" << i << "m" << "all";
    ss2 << "a" << i << "m" << "all" << "de2";
    analysisManager->CreateH1(ss1.str(), ss1.str(), qZ, 0, qZ - 1);
    analysisManager->CreateH1(ss2.str(), ss2.str(), qZ, 0, qZ - 1);
  }

  analysisManager->CreateH1("aAllmAll", "aAllmAll", qZ, 0, qZ - 1);
  analysisManager->CreateH1("aAllmAllde2", "aAllmAllde2", qZ, 0, qZ - 1);

  analysisManager->CreateH1("aAllmAllWithNM", "aAllmAll with zero mass/number particles with dE > 0", qZ, 0, qZ - 1);
  analysisManager->CreateH1("aAllmAllWithNMde2", "aAllmAllde2 with zero mass/number particles with dE > 0", qZ, 0, qZ - 1);

  G4String vrfTargeterLayers = cmdParam->getPriorityParamAsG4String("vrfTargeterLayers", "0,1");
  std::vector<std::string> rets;
  boost::split(rets, vrfTargeterLayers, boost::is_any_of(","));
  for (size_t i = 0; i < rets.size(); i++) {
    analysisCreateVrf4Layer(boost::lexical_cast<G4int>(rets[i]));
  }

}

void RunAction::analysisCreateVrf4Layer(G4int layerNo)
{
  std::stringstream ss;
  ss << "vrfLayer" << layerNo;
  G4String layerNoStr(ss.str());

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();
  G4double deMin = cmdParam->getPriorityParamAsG4Double("analysisVrfHistoDEMin", "0.0") * keV;
  G4double deMax = cmdParam->getPriorityParamAsG4Double("analysisVrfHistoDEMax", "1000") * keV;
  G4double betaMin = cmdParam->getPriorityParamAsG4Double("analysisVrfHistoBetaMin", "0.2");
  G4double betaMax = cmdParam->getPriorityParamAsG4Double("analysisVrfHistoBetaMax", "0.8");
  G4int AMax = cmdParam->getPriorityParamAsG4Int("analysisVrfHistoAMax", "30");
  G4int ZMax = cmdParam->getPriorityParamAsG4Int("analysisVrfHistoZMax", "30");

  analysisManager->CreateH1(layerNoStr + G4String("BetaCH"), "Beta spectrum of charged particles", 100, betaMin, betaMax);
  analysisManager->CreateH1(layerNoStr + G4String("BetaPerZCH"), "Beta per Z charged particles", 100, betaMin, betaMax);
  analysisManager->CreateH1(layerNoStr + G4String("de"), "Deposit spectrum (whole)", 100, deMin, deMax, "keV");
  analysisManager->CreateH1(layerNoStr + G4String("dedx"), "Raw de/dx spectrum", 500, 1, 1000);
  analysisManager->CreateH1(layerNoStr + G4String("dedxEmCal"), "de/dx spectrum (G4EmCalculator based)", 500, 1, 1000);
  analysisManager->CreateH1(layerNoStr + G4String("dedxPrim"), "Raw de/dx (primaries)", 300, 1, 1000);
  analysisManager->CreateH1(layerNoStr + G4String("dedxSecChg"), "Raw de/dx (charged secondaries)", 300, 1, 1000);

  analysisManager->CreateH1(layerNoStr + G4String("primKin"), "Kinetic spectrum of primaries, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");

  // @todo: refactor, get params from CMD
  analysisManager->CreateH1(layerNoStr + G4String("partNeutronSPC"), "Kinetic spectrum of neutron, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part1SPC"), "Kinetic spectrum of particles with aNumber == 1, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part2SPC"), "Kinetic spectrum of particles with aNumber == 2, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part3SPC"), "Kinetic spectrum of particles with aNumber == 3, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part4SPC"), "Kinetic spectrum of particles with aNumber == 4, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part5SPC"), "Kinetic spectrum of particles with aNumber == 5, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part6SPC"), "Kinetic spectrum of particles with aNumber == 6, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part7SPC"), "Kinetic spectrum of particles with aNumber == 7, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");
  analysisManager->CreateH1(layerNoStr + G4String("part8SPC"), "Kinetic spectrum of particles with aNumber == 8, MeV/nucleon", 500, 0 * MeV, 500 * MeV, "MeV");

  analysisManager->CreateH1(layerNoStr + G4String("AEvents"), "Number of events with particle A (de == 0.0 included)", AMax, 1, AMax);
  analysisManager->CreateH1(layerNoStr + G4String("ADep"), "(Number of events)*deposit with particle A", AMax, 1, AMax);

  //analysisManager->CreateH2(layerNoStr + G4String("KinDep"), "Kinetic energy versus dose", 500, 0 * MeV, 1 * MeV, 1000, 0.001*CLHEP::microgray, 0.1 * CLHEP::microgray, "MeV");
  analysisManager->CreateH2(layerNoStr + G4String("KinDep"), "Kinetic energy versus dose", 500, 0 * MeV, 500 * MeV, 1000, 76 * eV, 2 * MeV, "MeV", "MeV");
  analysisManager->CreateH2(layerNoStr + G4String("KinDepe-"), "Kinetic energy versus dose", 500, 0 * MeV, 5 * MeV, 1000, 76 * eV, 5 * MeV, "MeV", "MeV");

  analysisManager->CreateH1(layerNoStr + G4String("e-"), "Spectrum of e-", 500, 0 * MeV, 10 * MeV);
  //analysisManager->CreateH1(layerNoStr + G4String("gamma"), "Spectrum of gamma", 500, 0 * MeV, 10 * MeV);

  analysisManager->CreateH1(layerNoStr + G4String("PDGCharge"), "PDG Charge at layer", 41, -20, 20);

  G4bool use2DAngleVerifiers = cmdParam->getPriorityParamAsCppInt("analysisVrfHistoUse2DAngles", "0");
  if (use2DAngleVerifiers) {
    analysisManager->CreateH2(layerNoStr + G4String("A2CosX"), "A to direction.x", AMax, 1, AMax, 100, -1.0, 1.0);
    analysisManager->CreateH2(layerNoStr + G4String("A2CosY"), "A to direction.y", AMax, 1, AMax, 100, -1.0, 1.0);
    analysisManager->CreateH2(layerNoStr + G4String("A2CosZ"), "A to direction.z", AMax, 1, AMax, 100, -1.0, 1.0);
  }

}
