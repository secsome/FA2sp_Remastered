cmake_minimum_required(VERSION 3.28)

option(BUILD_PROJECT_IN_PARALLEL "Build project with multiple worker" ON)

project(MissionEditor)

enable_language(RC)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_ARCHITECTURE x86 CACHE STRING "Target architecture")
set_property(CACHE TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86)

if(DEFINED VCPKG_TARGET_TRIPLET)
    if(VCPKG_TARGET_TRIPLET MATCHES "^x64-.*")
        set(TARGET_ARCHITECTURE "x64")
    elseif(VCPKG_TARGET_TRIPLET MATCHES "^x86-.*")
        set(TARGET_ARCHITECTURE "x86")
    endif()
endif()
message(STATUS "Target architecture is ${TARGET_ARCHITECTURE}")

if(BUILD_PROJECT_IN_PARALLEL)
  if(MSVC)
    message(STATUS "Project will build with multiple worker.")
    add_compile_options(/MP)
  endif()
endif()

# Add more warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

if(MSVC)
  # Generate pdb files for release build
  add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
endif()

if(MSVC)
  # Disable whole program optimization for release build
  add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/GL->")
endif()

# General compiler flags
add_definitions(-DNOMINMAX)

# Use UTF-8 as the source character set
add_compile_options(/utf-8)

# Common include directories
include_directories(include)

add_subdirectory(3rdparty)
add_subdirectory(test)
add_subdirectory(src)
add_subdirectory(lib)
