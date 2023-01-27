cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(neutronEpi)

FIND_PACKAGE( Boost 1.61 REQUIRED)

find_package(Geant4 REQUIRED vis_all ui_all)
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include 
  ${Geant4_INCLUDE_DIR}  
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh ${PROJECT_SOURCE_DIR}/include/*.h)


add_library(NPParamReader SHARED NPParamReader.cc NPParamReader.hh)
if (WIN32) 
  target_compile_definitions(NPParamReader PUBLIC NPLIBRARY_BUILD_DLL)
endif()
target_compile_features(NPParamReader PRIVATE cxx_std_17)
target_link_libraries(NPParamReader PUBLIC  ${Geant4_LIBRARIES} )

add_executable(neutronEpi neutronEpi.cc  ${sources} ${headers})
target_link_libraries(neutronEpi PUBLIC
 ${Geant4_LIBRARIES} 
 NPParamReader
)