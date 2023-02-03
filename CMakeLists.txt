cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(g4epindorph)

find_package(Boost 1.61 REQUIRED system filesystem iostreams serialization)
find_package(Protobuf REQUIRED)
find_package(Geant4 REQUIRED ui_all)
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include 
  ${Geant4_INCLUDE_DIR}  
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
  ${Protobuf_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh ${PROJECT_SOURCE_DIR}/include/*.h)

#target_link_libraries(proto ${Protobuf_LIBRARIES})
#add_subdirectory(proto)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS NPPhaseSpaceData.proto)
add_library(proto ${PROTO_HDRS} ${PROTO_SRCS})
target_include_directories(proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


add_library(NPLibrary SHARED 
  NPParamReader.cc NPParamReader.hh 
  NPCad2Geant.cc NPCad2Geant.hh 
  NPBioLayerData.cc NPBioLayerData.hh 
  NPProtobufPhaseSpace.cc NPProtobufPhaseSpace.hh
  NPFastLayersData.cc NPFastLayersData.hh
)

target_compile_features(NPLibrary PRIVATE cxx_std_17)
#target_link_libraries(NPLibrary PUBLIC ${Geant4_LIBRARIES} $<$<BOOL:${Protobuf_FOUND}>:proto>)
target_link_libraries(NPLibrary PUBLIC ${Geant4_LIBRARIES} proto)
target_include_directories(NPLibrary PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
if (WIN32) 
  target_compile_definitions(NPLibrary PUBLIC NPLIBRARY_BUILD_DLL)
endif()

add_executable(g4epindorph neutronEpi.cc  ${sources} ${headers})
target_link_libraries(g4epindorph PUBLIC
 ${Geant4_LIBRARIES} 
 ${Protobuf_LIBRARIES}
 NPLibrary
)
target_include_directories(g4epindorph PUBLIC ${CMAKE_CURRENT_BINARY_DIR})