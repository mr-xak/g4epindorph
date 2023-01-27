#include "SteppingAction.hh"
#include "globals.hh"
#include "G4RunManager.hh"
#include "G4Types.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4Neutron.hh"
#include "G4Proton.hh"
#include "G4GenericIon.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Proton.hh"
#include "G4Positron.hh"

#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4AnalysisManager.hh"
#else
#include "g4root.hh"
#endif

#include "Run.hh"


SteppingAction::SteppingAction()
  :G4UserSteppingAction(), useGlobalProcessCounter(true)
{ }

SteppingAction::~SteppingAction() {

}

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  const G4StepPoint* endPoint = aStep->GetPostStepPoint();
  const G4StepPoint* startPoint = aStep->GetPreStepPoint();
  G4StepStatus stepStatus = endPoint->GetStepStatus();
  G4bool transmit = (stepStatus == fGeomBoundary || stepStatus == fWorldBoundary);
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  G4int hId;

  if (aStep->GetTrack()->GetParentID() == 0 && stepStatus == fWorldBoundary) {
    hId = analysisManager->GetH1Id("primaSpcOnWorldBoundary", false);
    if (hId != -1) {
      analysisManager->FillH1(hId, endPoint->GetKineticEnergy());
    }

    int pid = convertParticle2Pid(aStep->GetTrack()->GetParticleDefinition());
    G4double ene = aStep->GetTrack()->GetKineticEnergy() / MeV;
    G4double px = aStep->GetTrack()->GetPosition().x() / mm;
    G4double py = aStep->GetTrack()->GetPosition().y() / mm;
    G4double pz = aStep->GetTrack()->GetPosition().z() / mm;
    G4double ux = aStep->GetTrack()->GetMomentumDirection().x();
    G4double uy = aStep->GetTrack()->GetMomentumDirection().y();
    G4double uz = aStep->GetTrack()->GetMomentumDirection().z();

    Run* run = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    run->addParticle2PhaseSpace(pid, ene, px, py, pz, ux, uy, uz);
    //   Run* run = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  }
}

int SteppingAction::convertParticle2Pid(const G4ParticleDefinition* a)
{

  //if (ph.pid == 4)
  //  fParticleGun->SetParticleDefinition(neutron);
  //else if (ph.pid == 1)
  //  fParticleGun->SetParticleDefinition(gamma);
  //else if (ph.pid == 3)
  //  fParticleGun->SetParticleDefinition(electron);
  //else if (ph.pid == 1001)
  //  fParticleGun->SetParticleDefinition(proton);
  //else if (ph.pid == 6012)
  //  fParticleGun->SetParticleDefinition(c12);
  if (a == G4Neutron::NeutronDefinition()) {
    return 4;
  }
  else if (a == G4Gamma::GammaDefinition()) {
    return 1;
  }
  else if (a == G4Electron::ElectronDefinition()) {
    return 3;
  }
  else if (a == G4Positron::PositronDefinition()) {
    return 2;
  }
  else if (a == G4Proton::ProtonDefinition()) {
    return 1001;
  }
  else if (a == G4GenericIon::GenericIonDefinition()) {
    if (a->GetAtomicMass() == 12 && a->GetAtomicNumber() == 6) {
      return 6012;
    }
  }
  return 0;
}
