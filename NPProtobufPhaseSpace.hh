#ifndef NPProtobufPhaseSpace_hh
#define NPProtobufPhaseSpace_hh 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "NPPhaseSpaceData.pb.h"

class G4ParticleGun;
class G4Event;

class PGAProtobuf : public G4VUserPrimaryGeneratorAction
{
public:
  PGAProtobuf();
  ~PGAProtobuf();

public:
  virtual void GeneratePrimaries(G4Event*);

public:
  G4ParticleGun* GetParticleGun() { return fParticleGun; };

private:
  G4ParticleGun* fParticleGun;
  G4ThreeVector translationVector;
  G4RotationMatrix* rotationMatrix;
  NPLibrary::totPhaseSpaceMap phaseSpaceMap;
};
#endif