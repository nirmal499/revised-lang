cmake_minimum_required(VERSION 3.22.1)
project(lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(Boost_ROOT "/home/nbaskey/my_opt/boost_libs")
find_package(Boost 1.84 REQUIRED COMPONENTS)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

###### Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

set(EXECUTABLE_NAME "executable")

add_executable(${EXECUTABLE_NAME}
    app/main.cpp

    src/Evaluator.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/ExpressionSyntax.cpp
    src/Binder.cpp
    src/BoundNodePrinter.cpp
    src/BoundExpressionNode.cpp
    src/Symbol.cpp
    src/Lower.cpp
    src/Generator.cpp
    src/GenScope.cpp

)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE "include"
)

target_link_libraries(${EXECUTABLE_NAME} 
    PRIVATE ${llvm_libs}
)