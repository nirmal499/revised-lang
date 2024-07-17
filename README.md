# Trylang

## Overview

**Trylang** is a programming language designed to offer both interpretation and compilation capabilities.

## Features

- **Interpretation:** Interprets the entire Trylang code and provides the evaluated result.
- **Compilation to LLVM IR:** Compile Trylang code to LLVM Intermediate Representation (IR).
- **Data Types:** Includes support for essential data types such as `int`, `bool`, and `string`. Giving a type is optional.
- **Const:** Varaible declared with `var` are mutable and variable declared with `let` are not mutable.
- **Mandatory Return Values:** All functions must have a return value; if not explicitly provided, the default return type is `int`.

## Getting Started

### Prerequisites

- LLVM-14
- Boost-1.84

### Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/nirmal499/trylang.git
    cd trylang
    ```

2. Configure and build the project:
    ```sh
    ./run.sh project-configure-fresh
    ./run.sh project-build
    ```

### Usage

Make sure to save your file in the **source_file** directory present in the root folder in .txt file extension.

#### Running the Interpreter

To interpret and run Trylang code:
```sh
./run.sh project-run-exe <name_of_your_file_with_no_extension>
```
You will see the evaluted result after you run the above command.

You will find the compiled LLVM IR in **llv_ir** with the same name as you have provided above.

##### Example

The below code is saved in **source_file** directory, named as `main4.txt`
```
var a = "global";

function showA()
{
    // print is the inbuit function
    print(a);
    return 1;
}

showA();

a = "block";

showA();
```

To interpret and run the above Trylang code:
```sh
./run.sh project-run-exe main4
```

You will find the compiled LLVM IR in **llv_ir** directory with file named as `main4.txt`.

**A lot more features will be added in this project {WORK IN PROGRESS}**