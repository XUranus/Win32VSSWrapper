cmake_minimum_required (VERSION 3.8)
project ("vssclient")

# build executable cli
add_executable (vssclient "VssClient.cpp" "VssClient.h" "Demo.cpp")
target_link_libraries(vssclient VssApi)

# build static library
add_library(
    vsswrapper_static
    STATIC
    "VssClient.cpp" "VssClient.h"
)

target_link_libraries(vsswrapper_static VssApi)

# build dynamic library
add_library(
    vsswrapper
    SHARED
    "VssClient.cpp" "VssClient.h"
)

target_link_libraries(vsswrapper VssApi)
# to generate export library when build dynamic library, pass LIBRARY_EXPORT macro
target_compile_definitions(
    vsswrapper
    PRIVATE -DLIBRARY_EXPORT
)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vssclient PROPERTY CXX_STANDARD 17)
  set_property(TARGET vsswrapper_static PROPERTY CXX_STANDARD 17)
  set_property(TARGET vsswrapper PROPERTY CXX_STANDARD 17)
endif()