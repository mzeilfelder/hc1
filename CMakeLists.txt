cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo" FORCE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
	file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" HOVERCRAFT_VERSION)
	string(STRIP "${HOVERCRAFT_VERSION}" HOVERCRAFT_VERSION)
else()
    include(GetGitRevisionDescription)
    git_describe(HOVERCRAFT_VERSION)
endif()

project(hovercraft 
	VERSION ${HOVERCRAFT_VERSION}
	LANGUAGES CXX C
	HOMEPAGE_URL "http://www.github.com/michaelzeilfelder/hc1"
	DESCRIPTION "H-Craft Championship")

include(GNUInstallDirs)
include(FindPkgConfig)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

find_package(X11 REQUIRED)
find_package(Xrandr REQUIRED)
find_package(OpenGL REQUIRED)
find_package(libzip REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
find_package(ALUT REQUIRED)
find_package(Vorbis REQUIRED)
find_package(SDL REQUIRED)
find_package(Irrlicht REQUIRED)

file(GLOB_RECURSE HOVERCRAFT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HOVERCRAFT_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h )

if(WIN32)
	ENABLE_LANGUAGE(RC)
	set(HOVERCRAFT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.rc ${HOVERCRAFT_SOURCES})
endif()

add_compile_definitions(
	_IRR_SHARED_LIB_
	$<$<CONFIG:RELEASE>:NDEBUG=1>
	$<$<CONFIG:RELEASE>:HOVER_RELEASE>
	$<$<CONFIG:DEBUG>:DEBUG>
	$<$<CONFIG:DEBUG>:_DEBUG>
)

add_executable(${PROJECT_NAME} ${HOVERCRAFT_SOURCES} )
target_compile_options(${PROJECT_NAME} PUBLIC
	-Wall -W 
	-fno-strict-aliasing -Wno-unused-parameter 
	-fno-rtti -fno-exceptions
	$<$<CONFIG:DEBUG>:-g> 
	$<$<CONFIG:RELEASE>:-O3>
	$<$<CONFIG:RELEASE>:-fexpensive-optimizations>
)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	${PNG_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS}/freetype
	${ALUT_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} ${XRANDR_INCLUDE_DIRS}
	${Vorbis_INCLUDE_DIRS} ${SDL_INCLUDE_DIRS} ${IRRLICHT_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
	${PNG_LIBRARIES} ${Vorbis_LIBRARIES} 
	${FREETYPE_LIBRARIES} ${ALUT_LIBRARIES}
	${X11_LIBRARIES} ${XRANDR_LIBARY} OpenGL
	${SDL_LIBRARIES} ${IRRLICHT_LIBRARY})
