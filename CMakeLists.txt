cmake_minimum_required(VERSION 3.22.1)
project(lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.84 REQUIRED COMPONENTS program_options)

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
    # src/Generator.cpp
    # src/GenScope.cpp

)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE "include"
)

target_link_libraries(${EXECUTABLE_NAME} 
    PRIVATE Boost::program_options
)