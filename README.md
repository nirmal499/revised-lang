# Trylang

## Overview

**Trylang** is an interpreted language.

## Features

- **Interpretation:** Interprets the entire Trylang code and provides the evaluated result.
- **Types:** Includes support for essential types such as `int`, `bool`, and `string`. Giving a type is optional.
- **Const:** Variable declared with `var` are mutable and variable declared with `let` are not mutable.
- **Mandatory Return Values:** All functions must have a return value; if not explicitly provided, the default return type is `int`.
- **Control Flow:** `continue` and `break` statment is provided with `while` loop.
- **Function:** The last statement of a `function` must be a `return` statment. `function` must always return somthing. Incase return type of function is not provided than by default `int` will be used.
- **Global Variables:**  All variables that are declared outside a function are referred as Global Variable. And variables declared inside a function are referred as Local Variable.

## Getting Started

### Prerequisites

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
./run.sh project-run-exe <file_name>
```

##### Example

The below code is saved in **source_file** directory, named as `main14.txt`
```
function hi(name: string)
{
    var number = 45; // Local Variable
    print("All good " + name + " " + string(number));
    return 1;
}

let number = 7; // Global Variable
{
    var i = 0;  // Global Variable
    while(i <= 10)
    {
        if(i / 2 == 0)
        {
            print(string(i) + " skipping.");
            i = i + 1;
            continue;
        }

        print("Doing");

        if(i == number)
        {
            print("Found the target number " + string(i));
            break;
        }
        else
        {
            print(string(i) + " is not the target number.");
        }

        i = i + 1;
    }
}

hi("Nirmal");
```

To evaluate the above Trylang code:
```sh
./run.sh project-run-exe source_file/main14.txt
```

**A lot more features will be added in this project {WORK IN PROGRESS}**