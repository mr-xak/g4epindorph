#ifndef NPNewParamReader_hh
#define NPNewParamReader_hh 1

#ifdef WIN32
	#ifdef NPLIBRARY_BUILD_DLL
		#define NPLIBRARY_API __declspec(dllexport)
	#else
		#define NPLIBRARY_API __declspec(dllimport)
	#endif
#else
	#define NPLIBRARY_API
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

#include <vector>
#include <map>

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

#include "G4String.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

#ifdef G4MULTITHREADED
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#endif

namespace NPLibrary {
  namespace NPParamReader {

    struct NPLIBRARY_API NPBaseParams {
    public:
      bool visUse;
      bool isBasePhysics;
      int physNum;
      int RESeedType;
      G4String input;
      G4String output;
    };

    class NPLIBRARY_API NPParamReaderMain {

      // initializing methods
    public:
#ifdef G4MULTITHREADED
      NPParamReaderMain(bool isMaster=true);
#else
      NPParamReaderMain();
#endif
      virtual ~NPParamReaderMain();
      static NPParamReaderMain* Instance();

#ifdef G4MULTITHREADED
      static NPParamReaderMain* prMasterInstance;
      static NPParamReaderMain* prInstance;    
      //void merge(G4int rdNum, std::map<G4int, voxeledRunDataLayer> rdTarget);
#else
      static NPParamReaderMain* prInstance;
#endif

      void Init(); // @todo: rework

    public:
      void preGlobInit() {};
      void preWorkerInit() {};
      void preMasterInit() {};

#ifdef G4MULTITHREADED
      // @todo: merge call counter
      //void merge();
#endif

      // containters
    private:
      std::map<G4String, G4String> appParams;
      std::map<G4String, G4String> jsonParams;
      std::map<G4String, G4String> builtinParams;
      std::map<G4String, G4String> personalThreadParams;
      std::map<G4String, int> callCounter;
      std::vector<G4String> loneParams;
      NPBaseParams baseValues;

      // filler methods
    public:
      void fillAppJson();
      void fillGlobJson();
      void fillAppParams();

      void fillParamWise();

      void setCmdInput(int, char**);

      void setNoJson(bool _v) { noJson = _v; };

    private:
      void _fillJson(std::string);
      int _addAppParam(G4String, G4String);
      int _addPThreadParam(G4String, G4String);
      void _analyzeInput();
      bool noJson;
      std::string _appJsonPath;
      std::string _globJsonPath;

      // accessing methods

    public:
      bool isUseUI() { return baseValues.visUse; };
      bool isVisUse() { return baseValues.visUse; };
      G4String getInputMacro();
      bool getIsBasePhysics() { return baseValues.isBasePhysics; };
      int getPhysNum() { return baseValues.physNum; };
      int getPhysicsModifierValue() { return getPhysNum(); };
      int getRESeedType() { return baseValues.RESeedType; };
      G4String getPriorityParamAsG4String(G4String _k) { return getPriorityParamAsG4String(_k, "", true); };
      G4String getPriorityParamAsG4String(G4String _k, G4String _d) { return getPriorityParamAsG4String(_k, _d, true); };
      G4String getPriorityParamAsG4String(G4String, G4String, bool);
      inline int getPriorityParamAsCppInt(G4String _k, G4String _d) { return boost::lexical_cast<int>(getPriorityParamAsG4String(_k, _d)); };
      inline G4int getPriorityParamAsG4Int(G4String _k, G4String _d) { return boost::lexical_cast<G4int>(getPriorityParamAsG4String(_k, _d)); };
      inline double getPriorityParamAsCppDouble(G4String _k, G4String _d) { return boost::lexical_cast<double>(getPriorityParamAsG4String(_k, _d)); };
      inline G4double getPriorityParamAsG4Double(G4String _k, G4String _d) { return boost::lexical_cast<G4double>(getPriorityParamAsG4String(_k, _d)); };
      G4double getPriorityParamAsG4Double(G4String _k, G4double _d);

      G4ThreeVector getPriorityParamAsG4ThreeVector(G4String _k, G4ThreeVector _v);
      G4ThreeVector getPriorityParamAsG4ThreeVector(G4String _k) { return getPriorityParamAsG4ThreeVector(_k, G4ThreeVector(0, 0, 0)); };
      G4RotationMatrix* getPriorityParamAsG4RotationMatrixPointer(G4String _k);

      // backward compability
      G4String getPriorityParamAsString(G4String _k, G4String _d) {return getPriorityParamAsG4String(_k, _d, true); };

      // getMongoCompatibleParamsList();
      std::string getPriorityParamAsStdString(std::string _k, std::string _d) {return (getPriorityParamAsG4String(G4String(_k), G4String(_d), true)).c_str(); };

      // util methods
    public:
      std::vector<std::string> splitString(std::string, std::string);
      std::vector<G4double> splitString2G4double(std::string, std::string);

    private:
      int verboseLevel;
      //bool isGlobJsonRead;
      //bool isAppJsonRead;
      std::map<std::string, bool> isReadFlags;

      void _joinParamsArray();

    public:
      void printData() { printData(0); };
      void printData(int);
      void printStats();
      void setVerbose(int _v) { verboseLevel = _v; };
    };
  }; // NPParamReader

  using namespace NPParamReader;

}; // NPLibrary
#endif