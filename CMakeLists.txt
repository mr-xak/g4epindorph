cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(g4epindorph)

FIND_PACKAGE( Boost 1.61 REQUIRED)

find_package(Geant4 REQUIRED ui_all)
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include 
  ${Geant4_INCLUDE_DIR}  
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh ${PROJECT_SOURCE_DIR}/include/*.h)


add_library(NPLibraryMinimal SHARED 
  NPParamReader.cc NPParamReader.hh
  NPFastLayersData.cc NPFastLayersData.hh
)
if (WIN32) 
  target_compile_definitions(NPLibraryMinimal PUBLIC NPLIBRARY_BUILD_DLL)
endif()
target_compile_features(NPLibraryMinimal PRIVATE cxx_std_17)
target_link_libraries(NPLibraryMinimal PUBLIC  ${Geant4_LIBRARIES} )

add_executable(g4epindorph neutronEpi.cc  ${sources} ${headers})
target_link_libraries(g4epindorph PUBLIC
 ${Geant4_LIBRARIES} 
 NPLibraryMinimal
)