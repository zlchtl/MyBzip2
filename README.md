# MyBzip2

A command-line tool for compressing and decompressing files using the BZip2 compression algorithm.

## Requirements
* CMake 3.12 or higher
* BZip2 library
* C++20 compatible compiler

## Building
Install vcpkg package manager:
```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh  #Windows: bootstrap-vcpkg.bat
```
Install BZip2 library:
```bash
vcpkg install bzip2
```
Clone the repository:
```bash
git clone https://github.com/zlchtl/MyBzip2.git
cd MyBzip2
```
Create and navigate to the build directory:
```bash
mkdir build
cd build
```
Generate build files with CMake:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=[...]/vcpkg/scripts/buildsystems/vcpkg.cmake
```
Build the project:
```bash
cmake --build .
```

## Usage
```bash
MyBzip2 <command> <input_file> <output_file>

Commands:
  a    Compress input file
  e    Extract compressed file

Examples:
  ./MyBzip2 a input.txt compressed.bz2
  ./MyBzip2 e compressed.bz2 output.txt
```
