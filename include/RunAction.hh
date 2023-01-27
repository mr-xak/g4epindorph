#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"
//#include "NPLayerData.hh"
#include "Run.hh"
#include <map>
#include <vector>
#include <ctime>
#include "G4VProcess.hh"

#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4AnalysisManager.hh"
#else
#include "g4root.hh"
#endif


class Run;
class RunAction : public G4UserRunAction
{
public:
  RunAction();
  virtual ~RunAction();
  G4Run* GenerateRun();
  virtual void BeginOfRunAction(const G4Run* run);
  virtual void   EndOfRunAction(const G4Run* run);

  void createAnalysisData();
private:
  Run* fRun;

  clock_t beginRAAt;
  clock_t endRAAt;
  void analysisCreateVrf4Layer(G4int);
};

#endif