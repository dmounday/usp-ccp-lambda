
cmake_minimum_required(VERSION 3.18)

project(AWS_LAMBDA VERSION 1.1
        DESCRIPTION "Top level for all USP encode/decode lambdas."
  LANGUAGES CXX)
include(CMakePrintHelpers)
include(ExternalProject)
cmake_print_variables(CMAKE_PROJECT_NAME PROJECT_NAME)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)


#
# Get a specific version of google Protobuf
# Note: that the C++ file compiled by protoc must link to the same release of 
#       the header and libraries. 
# Note that the protobuf library is NOT shared.
set(PROTOBUF_ZIP 
    https://github.com/protocolbuffers/protobuf/releases/download/v3.13.0/protobuf-cpp-3.13.0.zip)
set(PROTOBUF_CONFIGURE <SOURCE_DIR>/configure --disable-shared --prefix=<INSTALL_DIR>)
ExternalProject_Add( protobuf-ext
  PREFIX protobuf
  URL ${PROTOBUF_ZIP}
  URL_HASH SHA1=ef71773fb9c5e27cf946cc6c0c6c82082dae8713
  UPDATE_COMMAND ""
  BUILD_ALWAYS false
  CONFIGURE_COMMAND ${PROTOBUF_CONFIGURE}
  BUILD_COMMAND ${MAKE}
)

ExternalProject_Get_Property(protobuf-ext SOURCE_DIR)
ExternalProject_Get_Property(protobuf-ext INSTALL_DIR)
set(PROTOBUF_INCLUDE ${INSTALL_DIR}/include)
set(PROTOBUF_LIB ${INSTALL_DIR}/lib)
set(PROTOC_EXECUTABLE ${INSTALL_DIR}/bin/protoc)
add_library(protobuf SHARED IMPORTED)
set_target_properties(protobuf PROPERTIES IMPORTED_LOCATION ${PROTOBUF_LIB}/libprotobuf.a)
cmake_print_variables(SOURCE_DIR INSTALL_DIR PROTOBUF_INCLUDE PROTOBUF_LIB)

add_subdirectory(USPmsg_rec_lib)
add_subdirectory(USP_MsgLib)
add_subdirectory(usp_encode)
add_subdirectory(usp_lambda)

