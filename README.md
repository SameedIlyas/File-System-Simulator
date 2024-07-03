# File System Simulators

This repository contains implementations of a file system simulator in both C++ and Python. Each implementation provides basic functionalities to create, read, write, and delete files and directories, as well as navigate through directories. The file system state is serialized and deserialized to a file for persistence.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building and Running the C++ Project](#building-and-running-the-c-project)
  - [Running the Python Project](#running-the-python-project)
- [Usage](#usage)

## Features

- Create and delete files and directories
- Change directories and navigate the directory structure
- Write to and read from files
- Move files between directories
- Persistent storage of file system state using serialization and deserialization

## Getting Started

### Prerequisites

For the C++ Project:
- A C++ compiler (e.g., g++)
- C++11 or higher

For the Python Project:
- Python 3.6 or higher

### Building and Running the C++ Project

To build and run the C++ project, use the following commands:

```bash
g++ -std=c++11 filesystem.cpp -o filesystem
./filesystem
```

### Running the Python Project

To run the Python project, use the following command:

```bash
python filesystem.py
```
## Usage
1. Run the application (C++ or Python).
2. Follow the menu options to interact with the file system:
  - List directory contents
  - Create new file
  - Delete file
  - Create new directory
  - Change directory
  - Move file
  - Write to file
  - Read from file
  - Write at fixed position in file
  - Read fixed number of characters from file
  - Exit
    
The file system state is saved to filesystem.dat (C++) or filesystem.json (Python) upon exit and loaded at startup.
