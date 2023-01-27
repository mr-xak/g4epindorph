#ifndef Run_h
#define Run_h 1

#include "G4Run.hh"
#include "G4Event.hh"

#include <vector>

#include <vector>

#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp> 
#include <boost/serialization/list.hpp> 
#include <boost/serialization/vector.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/collection_traits.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>

class PhaseSpace0 {
public:
  int pid;
  double ene;
  float x;
  float y;
  float z;
  float ux;
  float uy;
  float uz;
private:
  friend class boost::serialization::access;
  template <class Archive> void serialize(Archive& ar, const unsigned int version) {
    ar& pid& ene& x& y& z& ux& uy& uz;
  }
};

typedef std::map<G4int, G4double> tgD;

class Run : public G4Run {

public:
  Run();
  virtual ~Run();

  virtual void Merge(const G4Run*);

  void addParticle2PhaseSpace(int pid, G4double ene, G4double x, G4double y, G4double z, G4double ux, G4double uy, G4double uz);

  void addLETFulLData(G4double, G4double, G4double, G4double, G4double, G4int);

  void addDoseMap(tgD&);
  void addLetMap(tgD&);

  void EndOfRun();

private:
  tgD doseMap;
  tgD letMap;
  std::vector<PhaseSpace0> PhaseSpaceVectorTarget;

  G4double eneSum;
  G4double letBase1;
  G4double letBase2;
  G4double let2Avg;
  G4double dxSum;
  G4int qu;
};

//

#endif
