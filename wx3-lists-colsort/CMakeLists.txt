# CMake Lists (auto xwWidgets). 

# This names our executable (and for VS our project). 
set(APP wx3-lists-colsort) 

#-------------------
# Configure wxWidgets
#-------------------

include(FetchContent)



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
 
add_dependencies(${APP} wx::core wx::base)

# Link wxWidgets
target_link_libraries(${APP} LINK_PUBLIC wx::core wx::base)

# Link our libraries
# target_link_libraries (${APP} LINK_PUBLIC strings)


install(TARGETS ${APP}
	CONFIGURATIONS Release RelWithDebInfo Debug
	DESTINATION .
	)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Resources
	CONFIGURATIONS Release RelWithDebInfo Debug
	DESTINATION ./) 

message(STATUS "wx3-lists-colsort - DONE!")