#include "NPProtobufPhaseSpace.hh"
#include "globals.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "Randomize.hh"
#include "G4Version.hh"
#include "G4IonTable.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4AnalysisManager.hh"
#else
#include "g4root.hh"
#endif

#include "NPParamReader.hh"
#include "NPPhaseSpaceData.pb.h"

PGAProtobuf::PGAProtobuf()
  : G4VUserPrimaryGeneratorAction(),
  fParticleGun(0) {
  G4int n_particle = 1;
  fParticleGun = new G4ParticleGun(n_particle);

  NPLibrary::NPParamReaderMain* cmdParam = NPLibrary::NPParamReaderMain::Instance();

  G4String inpFName = cmdParam->getPriorityParamAsG4String("inputProtobufPhaseSpacePath", "test.pbphsp");
  {
    std::ifstream fnStream(inpFName.c_str(), std::ios::in | std::ios::binary);
    phaseSpaceMap.ParseFromIstream(&fnStream);
    //std::cout << phaseSpaceMap.ShortDebugString() << std::endl;
    fnStream.close();
  }

  G4cout << "Read " << phaseSpaceMap.phasespacemap_size() << " particles from " << inpFName << " . " << G4endl;

  translationVector = cmdParam->getPriorityParamAsG4ThreeVector("phaseSpaceTranslation");

  // @bug: here it applies X/Y/Z Cartesian grid rotations
  rotationMatrix = cmdParam->getPriorityParamAsG4RotationMatrixPointer("phaseSpaceRotation");
  //phaseSpaceMap
}

PGAProtobuf::~PGAProtobuf()
{
  delete fParticleGun;
}

void PGAProtobuf::GeneratePrimaries(G4Event* anEvent)
{

  G4double xpp, ypp, zpp;
  G4double xag, yag, zag;


  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  //G4ParticleDefinition* particle
  //  = particleTable->FindParticle("chargedgeantino");
  G4ParticleDefinition* proton
    = particleTable->FindParticle("proton");
  G4ParticleDefinition* neutron
    = particleTable->FindParticle("neutron");
  G4ParticleDefinition* gamma
    = particleTable->FindParticle("gamma");
  G4ParticleDefinition* electron
    = particleTable->FindParticle("e-");
  G4ParticleDefinition* positron
    = particleTable->FindParticle("e+");
  //G4int Z = 9, A = 18;
  //G4double ionCharge = 0.*eplus;
  //G4double excitEnergy = 0.*keV;
  G4ParticleDefinition* c12 = G4IonTable::GetIonTable()->GetIon(6, 12, 0.0);

  G4int rChoice = CLHEP::RandFlat::shootInt(phaseSpaceMap.phasespacemap_size());

  //std::vector<G4double> curParams = tvVals.find(rChoice)->second;
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4int rHid = analysisManager->GetH1Id("srcRandomChoice", false);
  if (rHid != -1) {
    analysisManager->FillH1(rHid, rChoice);
  }

  auto ph = phaseSpaceMap.phasespacemap(rChoice);
  if (ph.pid() == 4)
    fParticleGun->SetParticleDefinition(neutron);
  else if (ph.pid() == 1)
    fParticleGun->SetParticleDefinition(gamma);
  else if (ph.pid() == 3)
    fParticleGun->SetParticleDefinition(electron);
  else if (ph.pid() == 4)
    fParticleGun->SetParticleDefinition(positron);
  else if (ph.pid() == 1001)
    fParticleGun->SetParticleDefinition(proton);
  else if (ph.pid() == 6012)
    fParticleGun->SetParticleDefinition(c12);
  fParticleGun->SetParticleEnergy(ph.ene());

  xpp = ph.x();
  xpp += translationVector.x();
  ypp = ph.y();
  ypp += translationVector.y();
  zpp = ph.z();
  zpp += translationVector.z();
  G4ThreeVector a(xpp, ypp, zpp);
  a.rotate(rotationMatrix->phi(), rotationMatrix->theta(), rotationMatrix->psi());
  fParticleGun->SetParticlePosition(a);

  G4ThreeVector b(ph.ux(), ph.uy(), ph.uz());
  // @bug: here it reconverts it to rotate inputs via angles phi/theta/psi 
  b.rotate(rotationMatrix->phi(), rotationMatrix->theta(), rotationMatrix->psi());
  fParticleGun->SetParticleMomentumDirection(b);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}