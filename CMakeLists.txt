cmake_minimum_required(VERSION 3.22.1)
project(lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(Boost 1.84 QUIET COMPONENTS)

if(NOT Boost_FOUND)
    set(Boost_ROOT "/home/nbaskey/my_opt/boost_libs/")
    find_package(Boost 1.84 REQUIRED COMPONENTS)
endif()

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost 1.84 or later is required but was not found.")
else()
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

###### Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

set(EXECUTABLE_NAME "executable")

# ROOT_DIR is a macro defined in app/main.cpp
set(ROOT_DIR "${CMAKE_SOURCE_DIR}")
add_compile_definitions(ROOT_PATH="${ROOT_DIR}")

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