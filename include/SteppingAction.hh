#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4ParticleDefinition;


class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction();
  ~SteppingAction();

  virtual void UserSteppingAction(const G4Step*);

private:
  G4bool useGlobalProcessCounter;

  int convertParticle2Pid(const G4ParticleDefinition*);
};



#endif
