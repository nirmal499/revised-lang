cmake_minimum_required(VERSION 3.22.1)
project(lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(Boost_ROOT "/home/nbaskey/my_opt/boost_libs")
find_package(Boost 1.84 REQUIRED COMPONENTS)

set(EXECUTABLE_NAME "executable")

add_executable(${EXECUTABLE_NAME}
    app/main.cpp

    src/SyntaxTree.cpp
    src/Evaluator.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/ExpressionSyntax.cpp
)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE "include"
)