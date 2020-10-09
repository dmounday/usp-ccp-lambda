cmake_minimum_required(VERSION 3.18)

#project(USP_MsgLib 1.1
#        DESCRIPTION "USP Message handling library."
#  LANGUAGES CXX)
include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)


# Source files.
file(GLOB_RECURSE  SRC_FILE_LIST *.cpp *.h)

#cmake_print_variables(SRC_FILE_LIST)
add_library( USP_MsgLib STATIC ${SRC_FILE_LIST})
target_include_directories(USP_MsgLib PUBLIC src)
include_directories(${PROTOBUF_INCLUDE})
target_link_libraries(USP_MsgLib PUBLIC USPmsg_rec_lib )



