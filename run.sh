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
        echo "Usage: $0 project-run-exe <file_name>"
        exit 1
    fi

    ./build/executable -i $2
}

# Function to run the executable with gdb
project_run_debugger() {
    if [ -z $2 ]; then
        echo "Error: No filename provided."
        echo "Usage: $0 project-run-debugger <file_name>"
        exit 1
    fi

    gdb --args ./build/executable -i $2
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
        echo "Usage: $0 {project-configure-fresh|project-configure|project-build|project-run-exe <file_name>|project-run-debugger <file_name>}"
        exit 1
esac
