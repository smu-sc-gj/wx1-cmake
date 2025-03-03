# CMake Lists (auto xwWidgets). 

# This names our executable (and for VS our project). 
set(APP wx1-opengl) 

#-------------------
# Configure wxWidgets
#-------------------

include(FetchContent)


set(ONLY_LIBS NO CACHE BOOL "Build libs only" FORCE)
set(glew-cmake_BUILD_STATIC YES CACHE BOOL "Build the static glew library" FORCE)
set(glew-cmake_BUILD_SHARED NO CACHE BOOL "Build the shared glew library" FORCE)

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
    GIT_TAG glew-cmake-2.2.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE TRUE
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
    # FetchContent_MakeAvailable(glew)   
    FetchContent_Populate(glew)
    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

message(STATUS "GLEW_SOURCE_DIR: ${GLEW_SOURCE_DIR}")
message(STATUS "GLEW_BINARY_DIR: ${GLEW_BINARY_DIR}")
message(STATUS "GLEW_INCLUDE_DIRS: ${GLEW_INCLUDE_DIRS}")
message(STATUS "GLEW_LIBRARIES: ${GLEW_LIBRARIES}")

# Defaults are fine .. 
# set(wxBUILD_SAMPLES "OFF" CACHE STRING "SOME, ALL or OFF" FORCE)
set(wxBUILD_SHARED OFF CACHE STRING "Build shared or static libraries" FORCE)

FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets
    GIT_TAG v3.2.6
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE TRUE
)

set(ONLY_LIBS NO CACHE BOOL "Build libs only" FORCE)
set(glew-cmake_BUILD_STATIC YES CACHE BOOL "Build the static glew library" FORCE)
set(glew-cmake_BUILD_SHARED NO CACHE BOOL "Build the shared glew library" FORCE)

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
    GIT_TAG glew-cmake-2.2.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE TRUE
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
    # FetchContent_MakeAvailable(glew)   
    FetchContent_Populate(glew)
    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

if(NOT wxWidgets_POPULATED)
    FetchContent_MakeAvailable(wxWidgets)
endif()



file(GLOB GAME_FILES *.cpp)

IF (WIN32)
	add_executable(${APP} WIN32 ${GAME_FILES})
ELSE()
	add_executable(${APP} ${GAME_FILES})
ENDIF()

# Make sure dependent projects are compiled first!
# add_dependencies(${APP} strings) # my target(s)
 
add_dependencies(${APP} wx::core wx::base wx::gl)
add_dependencies(${APP} libglew_static)

# Link glew
# target_include_directories(${APP} PUBLIC ${GLEW_SOURCE_DIR}/include)
target_link_libraries(${APP} LINK_PUBLIC libglew_static)

# Link wxWidgets
target_link_libraries(${APP} LINK_PUBLIC wx::gl wx::core wx::base)

# Link our libraries
# target_link_libraries (${APP} LINK_PUBLIC strings)


install(TARGETS ${APP}
	CONFIGURATIONS Release RelWithDebInfo Debug
	DESTINATION .
	)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Resources
	CONFIGURATIONS Release RelWithDebInfo Debug
	DESTINATION ./) 

message(STATUS "Wx2 - DONE!")