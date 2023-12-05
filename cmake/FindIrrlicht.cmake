# - Find Irrlicht
# Find the Irrlicht headers and libraries
#
#  Irrlicht::Irrlicht - Imported target to use for building a library
#  Irrlicht_FOUND     - True if Irrlicht was found.
#

# Set up architectures (for windows) and prefixes (for mingw builds)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(IRRLICHT_LIB_PATH_SUFFIX ${CMAKE_INSTALL_LIBDIR})
else()
	set(IRRLICHT_LIB_PATH_SUFFIX ${CMAKE_INSTALL_LIBDIR})
endif()

if(IRRLICHT_PREFIX)
    set(IRRLICHT_ORIGPREFIXPATH ${CMAKE_PREFIX_PATH})
    if(IRRLICHT_ROOT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${IRRLICHT_ROOT_DIR}")
    endif()
    if(CMAKE_PREFIX_PATH)
        foreach(_prefix ${CMAKE_PREFIX_PATH})
            list(APPEND CMAKE_PREFIX_PATH "${_prefix}/${IRRLICHT_PREFIX}")
        endforeach()
    endif()
    if(MINGWSEARCH_PREFIXES)
        list(APPEND CMAKE_PREFIX_PATH ${MINGWSEARCH_PREFIXES})
    endif()
endif()

# Invoke pkgconfig for hints
#find_package(PkgConfig QUIET)
set(IRRLICHT_INCLUDE_HINTS)
set(IRRLICHT_LIB_HINTS)
#
#if(PKG_CONFIG_FOUND)
#	pkg_search_module(IRRLICHTPC QUIET Irrlicht)
#    if(IRRLICHTPC_INCLUDE_DIRS)
#        set(IRRLICHT_INCLUDE_HINTS ${IRRLICHTPC_INCLUDE_DIRS})
#    endif()
#    if(IRRLICHTPC_LIBRARY_DIRS)
#       set(IRRLICHT_LIB_HINTS ${IRRLICHTPC_LIBRARY_DIRS})
#    endif()
#endif()

include(FindPackageHandleStandardArgs)

find_library(IRRLICHT_LIBRARY
    NAMES
    Irrlicht
    HINTS
    ${IRRLICHT_LIB_HINTS}
    PATHS
    ${IRRLICHT_ROOT_DIR}
    ENV IRRLICHTDIR
    PATH_SUFFIXES ${CMAKE_INSTALL_LIBDIR} irrlicht ${IRRLICHT_LIB_PATH_SUFFIX})

set(_irrlicht_framework FALSE)
# Some special-casing if we've found/been given a framework.
# Handles whether we're given the library inside the framework or the framework itself.
if(APPLE AND "${IRRLICHT_LIBRARY}" MATCHES "(/[^/]+)*.framework(/.*)?$")
    set(_irrlicht_framework TRUE)
    set(IRRLICHT_FRAMEWORK "${IRRLICHT_LIBRARY}")
    # Move up in the directory tree as required to get the framework directory.
    while("${IRRLICHT_FRAMEWORK}" MATCHES "(/[^/]+)*.framework(/.*)$" AND NOT "${IRRLICHT_FRAMEWORK}" MATCHES "(/[^/]+)*.framework$")
        get_filename_component(IRRLICHT_FRAMEWORK "${IRRLICHT_FRAMEWORK}" DIRECTORY)
    endwhile()
    if("${IRRLICHT_FRAMEWORK}" MATCHES "(/[^/]+)*.framework$")
        set(IRRLICHT_FRAMEWORK_NAME ${CMAKE_MATCH_1})
        # If we found a framework, do a search for the header ahead of time that will be more likely to get the framework header.
        find_path(IRRLICHT_INCLUDE_DIR
            NAMES
            irrlicht.h # this file was introduced with IRRLICHT
            HINTS
            "${IRRLICHT_FRAMEWORK}/Headers/")
    else()
        # For some reason we couldn't get the framework directory itself.
        # Shouldn't happen, but might if something is weird.
        unset(IRRLICHT_FRAMEWORK)
    endif()
endif()

find_path(IRRLICHT_INCLUDE_DIR
    NAMES
    irrlicht.h # this file was introduced with IRRLICHT
    HINTS
    ${IRRLICHT_INCLUDE_HINTS}
    PATHS
    ${IRRLICHT_ROOT_DIR}
    ENV IRRLICHTDIR
    PATH_SUFFIXES include include/irrlicht irrlicht)

if(WIN32 AND IRRLICHT_LIBRARY)
    find_file(IRRLICHT_RUNTIME_LIBRARY
        NAMES
        Irrlicht.dll
        libIrrlicht.dll
        HINTS
        ${IRRLICHT_LIB_HINTS}
        PATHS
        ${IRRLICHT_ROOT_DIR}
        ENV IRRLICHTDIR
	PATH_SUFFIXES ${CMAKE_INSTALL_BINDIR} ${CMAKE_INSTALL_LIBDIR} ${IRRLICHT_LIB_PATH_SUFFIX})
endif()

if(IRRLICHT_PREFIX)
    # Restore things the way they used to be.
    set(CMAKE_PREFIX_PATH ${IRRLICHT_ORIGPREFIXPATH})
endif()

# handle the QUIETLY and REQUIRED arguments and set QUATLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Irrlicht
    DEFAULT_MSG
    IRRLICHT_LIBRARY
    IRRLICHT_INCLUDE_DIR
    ${IRRLICHT_EXTRA_REQUIRED})

if(Irrlicht_FOUND)
    if(NOT TARGET Irrlicht::Irrlicht)
        # Create IRRLICHT::IRRLICHT
        if(WIN32 AND IRRLICHT_RUNTIME_LIBRARY)
            set(IRRLICHT_DYNAMIC TRUE)
            add_library(Irrlicht::Irrlicht SHARED IMPORTED)
            set_target_properties(Irrlicht::Irrlicht
                PROPERTIES
                IMPORTED_IMPLIB "${IRRLICHT_LIBRARY}"
                IMPORTED_LOCATION "${IRRLICHT_RUNTIME_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${IRRLICHT_INCLUDE_DIR}"
            )
        else()
            add_library(Irrlicht::Irrlicht UNKNOWN IMPORTED)
            if(IRRLICHT_FRAMEWORK AND IRRLICHT_FRAMEWORK_NAME)
                # Handle the case that IRRLICHT is a framework and we were able to decompose it above.
                set_target_properties(Irrlicht::Irrlicht PROPERTIES
                    IMPORTED_LOCATION "${IRRLICHT_FRAMEWORK}/${IRRLICHT_FRAMEWORK_NAME}")
            elseif(_irrlicht_framework AND IRRLICHT_LIBRARY MATCHES "(/[^/]+)*.framework$")
		# Handle the case that Irrlicht is a framework and IRRLICHT_LIBRARY is just the framework itself.

                # This takes the basename of the framework, without the extension,
                # and sets it (as a child of the framework) as the imported location for the target.
                # This is the library symlink inside of the framework.
                set_target_properties(Irrlicht::Irrlicht PROPERTIES
                    IMPORTED_LOCATION "${IRRLICHT_LIBRARY}/${CMAKE_MATCH_1}")
            else()
                # Handle non-frameworks (including non-Mac), as well as the case that we're given the library inside of the framework
                set_target_properties(Irrlicht::Irrlicht PROPERTIES
                    IMPORTED_LOCATION "${IRRLICHT_LIBRARY}")
            endif()
            set_target_properties(Irrlicht::Irrlicht
                PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${IRRLICHT_INCLUDE_DIR}"
            )
        endif()

        if(APPLE)
            # Need Cocoa here, is always a framework
            find_library(IRRLICHT_COCOA_LIBRARY Cocoa)
            list(APPEND IRRLICHT_EXTRA_REQUIRED IRRLICHT_COCOA_LIBRARY)
            if(IRRLICHT_COCOA_LIBRARY)
                set_target_properties(Irrlicht::Irrlicht PROPERTIES
                        IMPORTED_LINK_INTERFACE_LIBRARIES ${IRRLICHT_COCOA_LIBRARY})
            endif()
        endif()
    endif()
    mark_as_advanced(IRRLICHT_ROOT_DIR)
endif()

mark_as_advanced(IRRLICHT_LIBRARY
    IRRLICHT_RUNTIME_LIBRARY
    IRRLICHT_INCLUDE_DIR
    IRRLICHT_SDLMAIN_LIBRARY
    IRRLICHT_COCOA_LIBRARY
    IRRLICHT_MINGW_LIBRARY
    IRRLICHT_MWINDOWS_LIBRARY)
