# Win32VSSWrapper
 > C++ API wrapper for Windows Volume Shadow Service

# Require
 - C++11
 - MSVC

## Build
```
git clone https://github.com/XUranus/Win32VSSWrapper.git
cd Win32VSSWrapper
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```

## Demo Usage
```
vssclient --list                          ----  list all shadows
vssclient delete <shadowID>               ----  delete the specified shadows
vssclient create <volumePath>             ----  create a shadow for specified volume
```