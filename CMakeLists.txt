# CMake Lists (auto gtest). 

cmake_minimum_required (VERSION 3.6.0)
 
set(PROJECT_NAME_STR WxWidgetsAndGlew)
project(${PROJECT_NAME_STR} C CXX)

#------------------------------
# Use more recent MSVC build 
# flags policy (abstraction)
#------------------------------
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/assets)

#from
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Set C++ standard (we're going with 2020). 
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_COMPILER_IS_GNUCXX)
  # New CMakeOnly
  # add_definitions(-Wall -ansi -Wno-deprecated -pthread)
  # Note: -ansi reinforces C90, will override c++11
  set(GNU_EXTRA_STUFF "-Wall -Wno-deprecated -Wno-multichar -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GNU_EXTRA_STUFF}")

  set(CMAKE_INSTALL_RPATH ${PROJECT_BINARY_DIR}/dist)
  
endif()

if(MSVC)
    #vc 2012 fix for vararg templates
    set(MSVC_COMPILER_DEFS "-D_VARIADIC_MAX=10")
endif()

#-------------------
# Configure Threads  
#-------------------
find_package(Threads REQUIRED)

# Recurse into the "maths" etc subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.

# Paul seems to have made these in order
# I'll try and follow that. 
add_subdirectory(wx1)

add_subdirectory(wx2-opengl)

#add_subdirectory(wx2-opengl-scribble2)

add_subdirectory(wx3-lists)

add_subdirectory(wx3-lists-colsort) 

if(MSVC)
  add_subdirectory(wx3-virtual-lists) 
endif()

message(STATUS "Top Level - Cmake is Done!")
