#ifndef NPFastLayersData_hh
#define NPFastLayersData_hh 1

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

#ifdef WIN32
#ifdef NPLIBRARY_BUILD_DLL
#define NPLIBRARY_API __declspec(dllexport)
#else
#define NPLIBRARY_API __declspec(dllimport)
#endif
#else
#define NPLIBRARY_API
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
      NPFastVoxelMain(bool isMaster);
#else
      NPFastVoxelMain();
#endif
      virtual ~NPFastVoxelMain();
      static NPLIBRARY_API NPFastVoxelMain* Instance();

#ifdef G4MULTITHREADED
      static NPFastVoxelMain* vxMasterInstance;
      static G4ThreadLocal NPFastVoxelMain* vxInstance;
#else
      static NPFastVoxelMain* vxInstance;
#endif

      void Init();
      void NPLIBRARY_API finalize(/*G4bool forced=false*/);
      void _merge(NPFastScoringMapPointer);

      void NPLIBRARY_API dumpDoseDepositWithError(std::string, myIntType);

      void NPLIBRARY_API setVoxelData(int, int, int);
      void NPLIBRARY_API setVoxelData(int, int, int, double);

      myIntType NPLIBRARY_API getNumFromXYZCoord(myIntType ix, myIntType iy, myIntType iz);

      void NPLIBRARY_API addData(myIntType, double);

      double NPLIBRARY_API getData(myIntType);
      double NPLIBRARY_API getAbsError(myIntType);

      inline void NPLIBRARY_API setCf(double _cf) { cf = _cf; };

    private:
      NPFastScoringMapPointer dosemap;
      int _xLim, _yLim, _zLim;
      bool isLimSet;
      double cf;
    }; // class NPFastVoxelMain
  };
};

#endif