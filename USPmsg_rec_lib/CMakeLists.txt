
cmake_minimum_required(VERSION 3.18)
include(CMakePrintHelpers)
project(USPmsg_rec_lib VERSION 1.1
        DESCRIPTION "Google Protobuf c++ compiler generated library for USP messages."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE PROTO_DEF_FILES  "*.proto")
file(MAKE_DIRECTORY src)

set(PB_INPUT_DIR "${PROJECT_SOURCE_DIR}")
set(PB_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/src")
# Create a list for all the *.cc and *.h file name that will be generated
# by protoc. 
list(APPEND CPP_SRC_LIST)
foreach (file ${PROTO_DEF_FILES})
   execute_process( 
    COMMAND ${PROTOC_EXECUTABLE} --proto_path=${PROJECT_SOURCE_DIR}/proto --cpp_out=${PB_OUTPUT_DIR} ${file}
    )
    get_filename_component(proto_name ${file} NAME_WE)
    list(APPEND CPP_SRC_LIST "${PB_OUTPUT_DIR}/${proto_name}.pb.cc"
                                "${PB_OUTPUT_DIR}/${proto_name}.pb.h"
                               )
endforeach()
#
# Set the properties to indicate that the *.cc and *.h files will be generated.   
set_source_files_properties( ${CPP_SRC_LIST} PROPERTIES GENERATED TRUE)
# Source files in executable.

add_library(USPmsg_rec_lib STATIC ${CPP_SRC_LIST} )
target_include_directories(USPmsg_rec_lib PUBLIC src)
include_directories(${PROTOBUF_INCLUDE})
target_link_libraries(USPmsg_rec_lib PUBLIC protobuf)


