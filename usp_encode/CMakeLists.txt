cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 11)
project(usp_encode LANGUAGES CXX)
find_package(aws-lambda-runtime )
find_package(aws-cpp-sdk-core )
find_package(aws-cpp-sdk-iot-data )
find_package(aws-cpp-sdk-states )


add_executable(${PROJECT_NAME} "main.cpp")
include_directories(${PROTOBUF_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE
 AWS::aws-lambda-runtime
 aws-cpp-sdk-core
 aws-cpp-sdk-iot-data
 USPmsg_rec_lib
 USP_MsgLib
 protobuf
 ) 
target_compile_features(${PROJECT_NAME} PRIVATE "cxx_std_11")
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra")

# this line creates a target that packages your binary and zips it up
aws_lambda_package_target(${PROJECT_NAME})
