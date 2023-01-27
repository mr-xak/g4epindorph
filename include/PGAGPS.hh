#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

//#include "NPPlanning.hh"

#include <vector>

class G4Event;
class PrimaryGeneratorMessenger;
class G4GeneralParticleSource;
//class DetectorConstruction;

class PGAGPS : public G4VUserPrimaryGeneratorAction
{
  public:
    PGAGPS();
   ~PGAGPS();
    virtual void GeneratePrimaries(G4Event*);

    
    G4GeneralParticleSource* GetParticleGun() { return fParticleGun; };

  private:
    G4GeneralParticleSource*        fParticleGun;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


