# usp-ccp-lambda
USP message encode/decode with AWS Lambda functions written in C++
# AWS Lambda conversion of USP messages

This is a project to encode and decode TR-369 USP messages. The messages are Google protobuf messages that are encoded/decoded from/to JSON for use by AWS functions.

The C++ encode/decode library is created by using the Google protoc compiler to generate the C++ source code based on the definition provided by the USP protocol description files. 

The CMakeList.txt will download a specific version of the protobuf package and build it with the configured compiler. This includes the protoc compiler which is used to process the protocol description files (usp-msg-1-1.proto, usp-record-1-1.proto). Building the protoc compiler ensures that the generated C++ code is compatible with the generated protobuf static library that is linked with the encode/decode lambdas.


### Requirements:



*   CMake 3.18
*   AWS Lambda C++ Runtime and its dependencies (See https://github.com/awslabs/aws-lambda-cpp)


### Build instructions:

Ensure that the AWS Lambda C++ Runtime and its dependencies are built as instructed.


```
    $ cd usp-cpp-lambda
    $ cmake -DCMAKE_PREFIX_PATH=<aws-libraries> -S . -B build
    $ cd build
    $ make

### Prefix notes
The cmake prefix may be something like:
    -DCMAKE_PREFIX_PATH='~/aws/aws_deps;~/aws/aws-lambda-lib' -S . -B build
    
