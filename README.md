# Win32VSSWrapper
 > C++ API wrapper for Windows Volume Shadow Service

# Require
 - C++17
 - MSVC

## Build
```
git clone https://github.com/XUranus/Win32VSSWrapper.git
cd Win32VSSWrapper
mkdir build
cd build
cmake .. -A x64 # for x64
cmake --build . --config Release
```

## Demo Usage
> require Administrator

```
vssclient list                                         ----  list all snapshots
vssclient delete <snapshotID>                          ----  delete the specified snapshot
vssclient delete all                                   ----  delete all snapshots
vssclient create <volumePath1> <volumePath2> ...       ----  create snapshot(set) for specified volumes
vssclient query <snapshotID>                           ----  query specified snapshot
vssclient mount <snapshotID> <path>                    ----  expose specified snapshot 
```
