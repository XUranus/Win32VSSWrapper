# Win32VSSWrapper
 > C++ API wrapper for Windows Volume Shadow Service

<div align="center">
<img src="https://github.com/XUranus/Win32VSSWrapper/actions/workflows/.github/workflows/cmake-single-platform.yml/badge.svg" alt="minijson" title="minijson">&thinsp;
<img src="https://img.shields.io/badge/-C++11-3F63B3.svg?style=flat&logo=C%2B%2B&logoColor=white" alt="C++14" title="C++ Standards Used: C++11">&thinsp;
<img src="https://img.shields.io/badge/-Windows-6E46A2.svg?style=flat&logo=windows-11&logoColor=white" alt="Windows" title="Supported Platform: Windows">&thinsp;
<img src="https://img.shields.io/badge/MSVC%202015+-flag.svg?color=555555&style=flat&logo=visual%20studio&logoColor=white" alt="MSVC 2015+" title="Supported Windows Compiler: MSVC 2015 or later">&thinsp;
</div>

# Require
 - C++17
 - MSVC2015+

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
vssclient umount <snapshotID>                          ----  umount specified snapshot
vssclient delset <snapshotSetID                        ----  delete snapshot set specified
vssclient mkwritable <snapshotID>                      ----  make snapshot writable (must have hardware provider)
```
