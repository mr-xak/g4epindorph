#include "DetectorSD.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4RunManager.hh"
#include "RunAction.hh"
#include "G4ParticleTypes.hh"
#include "G4IonTable.hh"
#include "G4GenericIon.hh"
#include "G4Analyser.hh"
#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4AnalysisManager.hh"
#else
#include "g4root.hh"
#endif
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "Run.hh"
#include "G4EmCalculator.hh"
#include "G4Neutron.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"

#include "NPFastLayersData.hh"

DetectorSD::DetectorSD(G4String name) : G4VSensitiveDetector(name), ownName(name), dxSum(0.0), letBase1(0.0),
letBase2(0.0), let2Avg(0.0), qu(0), eneSum(0.0), qu2(0)
{

}

DetectorSD::~DetectorSD() {

}

void DetectorSD::Initialize(G4HCofThisEvent*) {
  eneSum = 0.0;
  letBase1 = 0.0;
  letBase2 = 0.0;
  let2Avg = 0.0;
  dxSum = 0.0;
  qu = 0;
}

G4bool DetectorSD::ProcessHits(G4Step* aStep, G4TouchableHistory* aHist)
{
  if (aStep->GetTrack()->GetParticleDefinition() == G4Neutron::NeutronDefinition()) {
    return false;
  }
  if (aStep->GetTrack()->GetParticleDefinition() == G4Gamma::GammaDefinition()) {
    return false;
  }

  G4double de = aStep->GetTotalEnergyDeposit();
  G4double dx = aStep->GetStepLength();

  if (de > 0.0 && dx > 1 * nm /* && de < 15.0 * MeV*/) {
    eneSum += de;
    letBase1 += de * de / dx;
    dxSum += dx;
    G4EmCalculator emCal;
    //hId = analysisManager->GetH1Id("dedxTotalEM", false);
    //if (hId != -1) {
      //G4double cde = emCal.ComputeTotalDEDX(step->GetTrack()->GetKineticEnergy(), step->GetTrack()->GetParticleDefinition(), step->GetTrack()->GetTouchable()->GetVolume()->GetLogicalVolume()->GetMaterial());
      //analysisManager->FillH1(hId, vz, cde);
    //}
    G4double cde = emCal.ComputeTotalDEDX(aStep->GetTrack()->GetKineticEnergy(), aStep->GetTrack()->GetParticleDefinition(), aStep->GetTrack()->GetTouchable()->GetVolume()->GetLogicalVolume()->GetMaterial(), 20*um);
    letBase2 += de * cde;
    let2Avg += cde;
    qu += 1;
  }
  else {
    if (aStep->GetTrack()->GetParticleDefinition() == G4Electron::ElectronDefinition()) {
      G4double ede = aStep->GetTrack()->GetKineticEnergy();
      eneSum += ede;
      letBase1 += ede * ede / dx;
      dxSum += dx;
      G4EmCalculator emCal;
      G4double ecde = emCal.ComputeTotalDEDX(aStep->GetTrack()->GetKineticEnergy(), aStep->GetTrack()->GetParticleDefinition(), aStep->GetTrack()->GetTouchable()->GetVolume()->GetLogicalVolume()->GetMaterial(), 700 * um);
      letBase2 += ede * ecde;
      let2Avg += ecde;
      //qu += 1;
    }
    //else {
    //  G4cout << qu2 << ": Abnormal particle " << aStep->GetTrack()->GetParticleDefinition()->GetParticleName() << " having de " << de / MeV << " MeV and dx " << dx / um << " um " << G4endl;
    //  qu2 += 1;
    //}
    return false;
  }
  
  return true;
}

void DetectorSD::EndOfEvent(G4HCofThisEvent*)
{
  Run* run = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  // void Run::addLETFulLData(G4double _ene, G4double _let1, G4double _let2, G4double _letAvg, G4double _dx, G4int _qu) {
  run->addLETFulLData(eneSum, letBase1, letBase2, let2Avg, dxSum, qu);
  NPLibrary::NPFastLayerData::NPFastVoxelMain* vxFastData = NPLibrary::NPFastLayerData::NPFastVoxelMain::Instance();
  //vxFastData->addData(G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetNumberOfPrimaryVertex(), eneSum / MeV);
  vxFastData->addData(0, eneSum / MeV);

}
