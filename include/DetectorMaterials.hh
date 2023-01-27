#ifndef DetectorMaterials_h
#define DetectorMaterials_h 1
#include "globals.hh"

#include "G4NistManager.hh"
#include "NPParamReader.hh"

class G4Material;

/// <summary>
/// Class to define application materials
/// </summary>
class DetectorMaterials {
public:
  DetectorMaterials();
  ~DetectorMaterials();

  /// <summary>
  /// Function creating all materials used by application
  /// Further can be accessed via G4NistManager::Instance()->FindOrBuildMaterial()
  /// </summary>
  void DefineMaterials();

  /// <summary>
  /// Function returning material if it's name present in CMD or app.json params, otherwise returning second param
  /// </summary>
  /// <param name="name">name of material to find</param>
  /// <param name="def">Material to return if name not found</param>
  /// <returns>G4Material</returns>
  G4Material* getMaterialIfDefined(G4String name, G4Material* def);

  /// <summary>
  /// Function returning material if it's name present in CMD or app.json params, otherwise returning G4NistManager::FindOrBuildMaterial(second param);
  /// </summary>
  /// <param name="name">name of material to find</param>
  /// <param name="nName">Material to be found</param>
  /// <returns>G4Material</returns>
  //G4Material *getMaterialIfDefined(G4String name, G4String nName);

  /// <summary>
  /// Call 
  /// </summary>
  static DetectorMaterials* GetInstance();
private:
  bool isAlreadyCalled;
  static DetectorMaterials* instance;
};
#endif