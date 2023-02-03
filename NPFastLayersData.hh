#ifndef NPFastLayerData_hh
#define NPFastLayerData_hh 1

#include "G4Types.hh"
#include "G4String.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
#ifdef _WIN32
#include <boost/config/compiler/visualc.hpp>
#else
// @todo: check another variants
#include <boost/config/compiler/gcc.hpp>  
#include <inttypes.h>
#endif

#ifdef G4MULTITHREADED
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#endif

namespace NPLibrary {
  namespace NPFastLayerData {
#ifdef _WIN32
    typedef __int64 myIntType;
#else
    typedef int64_t myIntType;
#endif
    class NPFastScoringLayer {
    public:
      double depEnergy, depEnergy2;   // sum of dE, sum of dE*dE
      myIntType nEvents;

      void Nullify();
      NPFastScoringLayer();
      NPFastScoringLayer(double);
      NPFastScoringLayer(const NPFastScoringLayer&);
      virtual ~NPFastScoringLayer();

      void Update(double); // de
      void Update(NPFastScoringLayer);
      void Update(NPFastScoringLayer*);

      NPFastScoringLayer operator+ (const NPFastScoringLayer&);
      NPFastScoringLayer& operator+= (const NPFastScoringLayer&);

      double calculateError();

    }; // class NPFastScoringLayer

    typedef std::map<myIntType, NPFastScoringLayer*> NPFastScoringMapPointer;
    typedef std::pair<myIntType, NPFastScoringLayer*> NPFastScoringMapPointerPair;

    class NPFastVoxelMain {
    public:
#ifdef G4MULTITHREADED
      NPFastVoxelMain(bool isMaster = true);
#else
      NPFastVoxelMain();
#endif
      virtual ~NPFastVoxelMain();
      static NPFastVoxelMain* Instance();

#ifdef G4MULTITHREADED
      static NPFastVoxelMain* vxMasterInstance;
      static G4ThreadLocal NPFastVoxelMain* vxInstance;
#else
      static NPFastVoxelMain* vxInstance;
#endif

      void Init();
      void finalize(/*G4bool forced=false*/);
      void _merge(NPFastScoringMapPointer);

      void dumpDoseDepositWithError(std::string, myIntType);

      void setVoxelData(int, int, int);

      myIntType getNumFromXYZCoord(myIntType ix, myIntType iy, myIntType iz);

      void addData(myIntType, double);

    private:
      NPFastScoringMapPointer dosemap;
      int _xLim, _yLim, _zLim;
      bool isLimSet;
    }; // class NPFastVoxelMain
  };
};

#endif