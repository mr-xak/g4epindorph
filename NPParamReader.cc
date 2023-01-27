#include "NPParamReader.hh"

#include <fstream>

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
//#include <boost/filesystem.hpp>

using namespace NPLibrary;

#include "G4UnitsTable.hh"
#include "G4String.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

#ifdef G4MULTITHREADED
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#endif

#ifdef G4MULTITHREADED
namespace {
  G4Mutex prMainMutex = G4MUTEX_INITIALIZER;
}
NPParamReaderMain* NPParamReaderMain::prMasterInstance = 0;
NPParamReaderMain* NPParamReaderMain::prInstance = 0;
#else
NPParamReaderMain* NPParamReaderMain::prInstance = 0;
#endif

#ifdef G4MULTITHREADED
NPParamReaderMain::NPParamReaderMain(bool isMaster) {
  if ( isMaster ) prMasterInstance = this;
  prInstance = this;
}
NPParamReaderMain::~NPParamReaderMain() {
}
#else
NPParamReaderMain::NPParamReaderMain() {
}

NPParamReaderMain::~NPParamReaderMain() {
}
#endif

#ifdef G4MULTITHREADED
NPParamReaderMain* NPParamReaderMain::Instance()
{
  if ( prInstance == 0 ) {
    G4bool isMaster = ! G4Threading::IsWorkerThread();
    //G4bool isMaster = G4Threading::IsMasterThread();
    prInstance = new NPParamReaderMain(isMaster);
    if (isMaster) prMasterInstance->Init();
    prInstance->Init();
  }
  
  return prInstance;
}    
#else
NPParamReaderMain* NPParamReaderMain::Instance()
{
  if ( prInstance == 0 ) {
    prInstance = new NPParamReaderMain();
    prInstance->Init();
  }
  return prInstance;
}
#endif

std::vector<std::string> NPParamReaderMain::splitString(std::string _str, std::string _delim) {
  std::vector<std::string> strs;
  boost::split(strs, _str, boost::is_any_of(_delim));
  return strs;
}

std::vector<G4double> NPParamReaderMain::splitString2G4double(std::string _str, std::string _delim) {
  std::vector<std::string> strs = splitString(_str, _delim);
  std::vector<G4double> ret;
  for(size_t i = 0; i < strs.size(); i++) {
    ret.push_back(boost::lexical_cast<G4double>(strs[i]));
  }
  return ret;
}

void NPParamReaderMain::fillAppJson() {
  if (noJson) {
    return;
  }
  if ( _appJsonPath.length() < 1 ) {
    _fillJson("app.json");
    isReadFlags.insert(std::pair<std::string, bool>("appJson", true));
  } else {
    _fillJson(_appJsonPath);
    isReadFlags.insert(std::pair<std::string, bool>("userJson", true));
  }
}
void NPParamReaderMain::fillGlobJson() {
  if (noJson)
    return;
  _fillJson("glob.json");
  isReadFlags.insert(std::pair<std::string, bool>("globJson", true));
}

void NPParamReaderMain::_fillJson(std::string pth) {
  std::stringstream ss;
  if (pth.length() > 1) {
    ss << pth;
  }
  std::cout<<"--- Info: reading json data from \""<<ss.str()<<"\" ----"<<std::endl;
  std::ifstream fCheck(ss.str());
  if (!fCheck.good()) {
    return;
  }
  if (true) {
  //if (boost::filesystem::exists( ss.str() ) ) {
    boost::property_tree::ptree doc;
    boost::property_tree::read_json(ss.str(), doc);  
    //G4cout<<"App data from "<<ss.str()<<" (* - overrided by cmd): "<<G4endl;
    BOOST_FOREACH (boost::property_tree::ptree::value_type& opt, doc) {
      G4cout<<"\t"<<opt.first<<" "<<opt.second.data();
      //if (getOtherParamAsString(G4String(opt.first), "") != "") {
        //G4cout<<" * ";
      //}
      if (appParams.count(opt.first) != 0) {
        G4cout<<" * ";
      }
      G4cout<<G4endl;
      if (jsonParams.count(opt.first) == 0) {
        jsonParams.insert(std::pair<G4String, G4String>(opt.first, opt.second.data()));
      } else {
        G4cerr << opt.first << "already had value" << jsonParams.find(opt.first)->second <<". Using new value: "<<opt.second.data()<<G4endl;
        jsonParams.find(opt.first)->second = opt.second.data();
      }
    }
    //isAppJsonRead = true;
  }

#ifdef G4MULTITHREADED
  if (G4Threading::IsWorkerThread()) {
    G4AutoLock lMut(&prMainMutex);
    for (auto it(jsonParams.begin()); it != jsonParams.end(); ++it) {
      if (prMasterInstance->jsonParams.count(it->first) == 0) {
        prMasterInstance->jsonParams.insert(std::pair<G4String, G4String>(it->first, it->second));
      }
    }
    lMut.unlock();
  }
#endif
}


/*
 Returning found value prior to order (builtin < json < cmd), also counting calls for each item
 @param item: item to find
 @type item: G4String
 @param dVal: default value to be used if item not found
 @type dVal: G4String
 @param _lookInMaster: 
 @type _lookInMaster: bool
 @rtype: G4String
*/
G4String NPParamReaderMain::getPriorityParamAsG4String(G4String item, G4String dVal, bool _lookInMaster) {
  if (callCounter.count(item) == 0) {
    callCounter.insert(std::pair<G4String, int>(item, 1));
  } else {
    callCounter.find(item)->second += 1;
  }

  if (appParams.count(item) != 0) {
    return appParams.find(item)->second;
  }

#ifdef G4MULTITHREADED
  if (G4Threading::IsWorkerThread()) {
    if (personalThreadParams.count(item) != 0) {
      return personalThreadParams.find(item)->second;
    }
  }
#endif

#ifdef G4MULTITHREADED
  if (_lookInMaster) {
    if (prMasterInstance->appParams.count(item) != 0) {
      appParams.insert(std::pair<G4String, G4String>(item, prMasterInstance->appParams.find(item)->second));
      return prMasterInstance->appParams.find(item)->second;
    }
  }
#endif

  if (jsonParams.count(item) != 0) {
    return jsonParams.find(item)->second;
  }

#ifdef G4MULTITHREADED
  if (_lookInMaster) {
    if (prMasterInstance->jsonParams.count(item) != 0) {
      jsonParams.insert(std::pair<G4String, G4String>(item, prMasterInstance->jsonParams.find(item)->second));
      return prMasterInstance->jsonParams.find(item)->second;
    }
  }
#endif

  if (builtinParams.count(item) != 0) {
    G4String rVal = builtinParams.find(item)->second;
    if (rVal != dVal) {
      if (dVal == "") {
        G4cerr<<" !! Item "<<item<<" found in previous calls with value \""<<rVal<<"\" but now is empty. Returning found value. (Note: OK if you see this after program completed)"<<G4endl;
        return rVal;
      } else {
        G4cerr<<" !! Item "<<item<<" found in previous calls with value \""<<rVal<<"\" but now suggested value is \""<<dVal<<"\". Returning suggested value (old value NOT updated)."<<G4endl;
        return dVal;
      }
    } else {
      return dVal;
    }
  } else {
#ifdef G4MULTITHREADED
    if (_lookInMaster) {
      if (prMasterInstance->builtinParams.count(item) != 0) {
        builtinParams.insert(std::pair<G4String, G4String>(item, prMasterInstance->builtinParams.find(item)->second));
        return prMasterInstance->builtinParams.find(item)->second;
      } else {
        G4AutoLock lMut(&prMainMutex);
        prMasterInstance->builtinParams.insert(std::pair<G4String, G4String>(item, dVal));
        lMut.unlock();
      }
    }
#endif
    builtinParams.insert(std::pair<G4String, G4String>(item, dVal));
    return dVal;
  }

  return dVal;
  //return "";
}

G4ThreeVector NPLibrary::NPParamReader::NPParamReaderMain::getPriorityParamAsG4ThreeVector(G4String _k, G4ThreeVector defaultVector) {
  G4ThreeVector tg = defaultVector;
  G4String x = getPriorityParamAsG4String(_k + G4String("X"), "none");
  G4String y = getPriorityParamAsG4String(_k + G4String("Y"), "none");
  G4String z = getPriorityParamAsG4String(_k + G4String("Z"), "none");
  G4String xyz = getPriorityParamAsG4String(_k + G4String("XYZ"), "none");
  if ("none" == x && "none" == y && "none" == z && "none" == xyz) {
    return defaultVector;
  } else {
    if ("none" != xyz) {
      std::vector<G4double> spl1 = splitString2G4double(xyz, ",");
      tg.setX(spl1[0] * mm);
      tg.setY(spl1[1] * mm);
      tg.setZ(spl1[2] * mm);
      return tg;
    }
    if ("none" != x) {
      tg.setX(boost::lexical_cast<G4double>(x) * mm);
    }
    if ("none" != y) {
      tg.setY(boost::lexical_cast<G4double>(y) * mm);
    }
    if ("none" != z) {
      tg.setZ(boost::lexical_cast<G4double>(z) * mm);
    }
    return tg;
  }
}

G4RotationMatrix * NPLibrary::NPParamReader::NPParamReaderMain::getPriorityParamAsG4RotationMatrixPointer(G4String _k)
{
  G4RotationMatrix *rtm = new G4RotationMatrix();
  G4String x = getPriorityParamAsG4String(_k + G4String("agX"), "none");
  G4String y = getPriorityParamAsG4String(_k + G4String("agY"), "none");
  G4String z = getPriorityParamAsG4String(_k + G4String("agZ"), "none");
  G4String xyz = getPriorityParamAsG4String(_k + G4String("agXYZ"), "none");
  if ("none" == x && "none" == y && "none" == z && "none" == xyz) {
    return rtm;
  } else {
    if ("none" != xyz) {
      std::vector<G4double> spl1 = splitString2G4double(xyz, ",");
      rtm->rotateX(spl1[0] * deg);
      rtm->rotateY(spl1[1] * deg);
      rtm->rotateZ(spl1[2] * deg);
      return rtm;
    }
    if ("none" != x) {
      rtm->rotateX(boost::lexical_cast<G4double>(x) * deg);
    }
    if ("none" != y) {
      rtm->rotateY(boost::lexical_cast<G4double>(y) * deg);
    }
    if ("none" != z) {
      rtm->rotateZ(boost::lexical_cast<G4double>(z) * deg);
    }
    return rtm;
  }
}

G4double NPLibrary::NPParamReader::NPParamReaderMain::getPriorityParamAsG4Double(G4String _k, G4double _d)
{
  std::stringstream ss;
  ss << _d;
  return boost::lexical_cast<G4double>(getPriorityParamAsG4String(_k, ss.str()));
}

void NPParamReaderMain::_joinParamsArray() {
}

void NPParamReaderMain::Init() {
#ifdef G4MULTITHREADED
  if (G4Threading::IsWorkerThread() && prMasterInstance->isReadFlags.count("cmdParsed") && prMasterInstance->isReadFlags.find("cmdParsed")->second) {
    baseValues.visUse = prMasterInstance->baseValues.visUse;
    baseValues.physNum = prMasterInstance->baseValues.physNum;
    baseValues.isBasePhysics = prMasterInstance->baseValues.isBasePhysics;
    baseValues.RESeedType = prMasterInstance->baseValues.RESeedType;
    baseValues.input = prMasterInstance->baseValues.input;
    baseValues.output = prMasterInstance->baseValues.output;
    noJson = prMasterInstance->noJson;
    _appJsonPath = prMasterInstance->_appJsonPath;
    return;
  }
#endif
  baseValues.visUse = false;
  baseValues.physNum = 1;
  baseValues.isBasePhysics = true;
  baseValues.RESeedType = 1;
  baseValues.input = "run.mac";
  baseValues.output = "out";
  noJson = false;
  _appJsonPath = "";
#ifdef G4MULTITHREADED
  prMasterInstance->baseValues.visUse = false;
  prMasterInstance->baseValues.physNum = 1;
  prMasterInstance->baseValues.isBasePhysics = true;
  prMasterInstance->baseValues.RESeedType = 1;
  prMasterInstance->baseValues.input = "run.mac";
  prMasterInstance->baseValues.output = "out";
  prMasterInstance->noJson = false;
  prMasterInstance->_appJsonPath = "";

  if (prMasterInstance->isReadFlags.count("cmdParsed") && prMasterInstance->isReadFlags.find("cmdParsed")->second) {

  }
#endif
}

G4String NPParamReaderMain::getInputMacro() {
#ifdef G4MULTITHREADED
  if (verboseLevel > 1) {
    G4cout << "Master: " << prMasterInstance->baseValues.input << " Local: " << baseValues.input << " (now master: "<<G4Threading::IsWorkerThread()<<")"<< G4endl;
  }
  return prMasterInstance->baseValues.input;
#endif
  return baseValues.input;
}

void NPParamReaderMain::setCmdInput(int _v, char** _c) {
  bool tvXFlag = false;

  appParams.insert(std::pair<G4String, G4String>("applicationExecutableName", G4String(_c[0])));

  for (int i = 0; i < _v; i++) {
    G4String cParam = G4String(_c[i]);
    std::string cParamStd = std::string(_c[i]);

    // -tvX -> put to buffer, wait next occurence
    if ("-tvX" == cParam) {
      tvXFlag = true;
      continue;
    }

    if ('-' != cParam[0]) {
      loneParams.push_back(cParam);
      continue;
    }
    if (i+1 < _v && boost::find_first(_c[i], "-") && boost::find_first(_c[i+1], "--")) {
      loneParams.push_back(cParam);
      continue;
    }
    G4String _key, _val;
    if (strchr(cParam.c_str(), '=')) {
      std::vector<std::string> strs;
      boost::split(strs,cParamStd,boost::is_any_of("="));
      _key = G4String(strs[0]);
      _val = G4String(strs[1]);
    } else {
      _key = cParam;
      _val = G4String(_c[i+1]);
      i+= 1;
    }

    if (tvXFlag) {
      _addPThreadParam(_key, _val);
    } else {
      _addAppParam(_key, _val);
      if (boost::find_first(_key, "--")) {
        boost::erase_all(_key, "--");
        _addAppParam(_key, _val);
      }
    }

    tvXFlag = false;
  }

  isReadFlags.insert(std::pair<std::string, bool>("cmd", true));

  // readGlobJson();
  // readAppJson();

  _analyzeInput();
}

void NPParamReaderMain::_analyzeInput() {
  for (int i = 0; i < loneParams.size(); i++) {
    if (loneParams[i] == "u") {
      baseValues.visUse = true;
    }
  }
  for( auto it(appParams.begin()); it != appParams.end(); ++it) {
    if ("-u" == it->first && boost::lexical_cast<bool>(it->second)) {
      baseValues.visUse = true;
      if (verboseLevel > 1) {
        G4cout<<"Use visualization"<<G4endl;
      }
    }
    if ("-m" == it->first ) {
      baseValues.input = it->second;
    }
    if ("-ph" == it->first) {
      int phNum = boost::lexical_cast<int>(it->second);
      if (phNum < 0) {
        baseValues.isBasePhysics = true;
        baseValues.physNum = std::abs(phNum);
      } else {
        baseValues.isBasePhysics = false;
        baseValues.physNum = phNum;
      }
    }
    if ("-seedType" == it->first) {
      baseValues.RESeedType = boost::lexical_cast<int>(it->second);
    }
    if ("--binaryFileName" == it->first) {
      baseValues.output = it->second;
    }
    if ("--appJson" == it->first) {
      _appJsonPath = it->second.c_str();
    }
  }

  isReadFlags.insert(std::pair<std::string, bool>("cmdParsed", true));

#ifdef G4MULTITHREADED
  if (G4Threading::IsWorkerThread()) {
    if (prMasterInstance->isReadFlags.count("cmdParsed") && prMasterInstance->isReadFlags.find("cmdParsed")->second) {
      G4AutoLock lMut(&prMainMutex);

      lMut.unlock();
    }
  }
#endif
}

int NPParamReaderMain::_addAppParam(G4String _key, G4String _val) {
  if (appParams.count(_key) == 0) {
    appParams.insert(std::pair<G4String, G4String>(_key, _val));
#ifdef G4MULTITHREADED
    if (prMasterInstance->appParams.count(_key) == 0) {
      prMasterInstance->appParams.insert(std::pair<G4String, G4String>(_key, _val));
    }
#endif
    return 0;
  } else {
    G4cerr<<"Key \""<<_key<<"\" duplicated. Old: \""<<appParams.find(_key)->second<<"\", new: \""<<_val<<"\". Using old."<<G4endl;
    return 1;
  }
}
int NPParamReaderMain::_addPThreadParam(G4String _key, G4String _val) {
  if (personalThreadParams.count(_key) == 0) {
    personalThreadParams.insert(std::pair<G4String, G4String>(_key, _val));
    return 0;
  } else {
    G4cerr<<"Key \""<<_key<<"\" duplicated. Old: \""<<personalThreadParams.find(_key)->second<<"\", new: \""<<_val<<"\". Using old."<<G4endl;
    return 1;
  }
}

void NPParamReaderMain::printData(int _l=0) {
  if (verboseLevel > 1) {
    G4cout<<"====== NPParamReaderMain output ======"<<G4endl;
    G4cout<<"Level App params: "<<G4endl;
    for( auto it(appParams.begin()); it != appParams.end(); ++it) {
      G4cout << "Param \""<<it->first<<"\" parsed as \""<<it->second<<"\" (dummy)"<<G4endl;
    }
    G4cout<<"Level Json params: "<<G4endl;
    for( auto it(jsonParams.begin()); it != jsonParams.end(); ++it) {
      G4cout << "Param \""<<it->first<<"\" parsed as \""<<it->second<<"\" (dummy app.json)"<<G4endl;
    }
  }
}

void NPParamReaderMain::printStats() {
  if (verboseLevel > 1) {
    std::cout<<"===== NPParamReaderMain stats ====="<<std::endl;
    for (auto it(callCounter.begin()); it != callCounter.end(); ++it) {
      std::cout<<it->first<<"(\""<<getPriorityParamAsG4String(it->first).c_str()<<"\") used "<<it->second<<" times."<<std::endl;
    }
    std::cout<<"===== EOF NPParamReaderMain stats ====="<<std::endl;
  }
}




