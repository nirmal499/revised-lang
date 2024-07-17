#!/bin/bash

# Function to configure the project from scratch
project_configure_fresh() {
    rm -rf build
    mkdir build
    cmake -B build -S .
}

# Function to configure the project
project_configure() {
    cmake -B build -S .
}

# Function to build the project
project_build() {
    cmake --build build
}

# Function to run the executable
project_run_exe() {
    if [ -z $2 ]; then
        echo "Error: No filename provided."
        echo "Usage: $0 project-run-exe <name_of_your_file_with_no_extension>"
        exit 1
    fi
    ./build/executable $2.txt
}

# Function to run the executable with gdb
project_run_debugger() {
    if [ -z $2 ]; then
        echo "Error: No filename provided."
        echo "Usage: $0 project-run-debugger <name_of_your_file_with_no_extension>"
        exit 1
    fi
    gdb --args ./build/executable $2.txt
}

# Main script logic to handle arguments
case "$1" in
    project-configure-fresh)
        project_configure_fresh
        ;;
    project-configure)
        project_configure
        ;;
    project-build)
        project_build
        ;;
    project-run-exe)
        project_run_exe $@
        ;;
    project-run-debugger)
        project_run_debugger $@
        ;;
    *)
        echo "Usage: $0 {project-configure-fresh|project-configure|project-build|project-run-exe <name_of_your_file_with_no_extension>|project-run-debugger <name_of_your_file_with_no_extension>}"
        exit 1
esac
