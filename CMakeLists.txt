cmake_minimum_required (VERSION 3.8)

project ("vssclient")

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
add_executable (vssclient "VssClient.cpp" "VssClient.h" "Demo.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vssclient PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(vssclient VssApi)