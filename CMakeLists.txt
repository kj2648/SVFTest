cmake_minimum_required(VERSION 3.4.3)

set(CMAKE_CXX_STANDARD 11)

#set(LLVM_DIR "/home/kj-dev/projects/llvm-project/build")

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

if (EXISTS "${LLVM_DIR}")
    message(STATUS "LLVM_DIR: ${LLVM_DIR}")
    message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")
    message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
endif()

set(SVF_HOME "/home/kj-dev/projects/SVF")
set(SVF_HEADER "${SVF_HOME}/include")
set(SVF_LIB "${SVF_HOME}/Release-build/lib")

if (EXISTS "${SVF_HEADER}" AND EXISTS "${SVF_LIB}")
    message(STATUS "SVF_HEADER: ${SVF_HEADER}")
    message(STATUS "SVF_LIB: ${SVF_LIB}")

    include_directories(${SVF_HEADER})
    link_directories(${SVF_LIB} ${SVF_LIB}/CUDD)
endif()

llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support passes )

add_executable( SVFTest main.cpp )

target_link_libraries( SVFTest
    LLVMSvf
    LLVMCudd
    ${llvm_libs} )

#set_target_properties( SVFTest PROPERTIES
                       #RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )