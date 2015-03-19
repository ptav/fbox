# Find TinyXML library
#
# Once found it will define:
# TinyXML_FOUND              - True if library is found
# TinyXML_INCLUDE_DIR        - Include directory
# TinyXML_LIBRARIES          - Libraries
#
# TinyXML_LIBRARIES_DEBUG    - Debug libraries
# TinyXML_LIBRARIES_RELEASE  - Release libraries

# Copyright 2014 Pedro A.C. Tavares
# Distributed under the Liomited GNU Public License (LGPL)

include(LibFindMacros)
include(FBoxHelperMacros)

fbox_build_sufix(BUILD_SUFIX)

# Dependencies
# libfind_package(TinyXML <dep>)

# Get hints about paths
libfind_pkg_check_modules(TinyXML_PKGCONF TinyXML)

# Includes
find_path(TinyXML_INCLUDE_DIR NAMES tinyxml.h PATHS ${TinyXML_PKGCONF_INCLUDE_DIRS})

# Library
find_library(TinyXML_LIBRARY_RELEASE tinyxml${BUILD_SUFIX} tinyxml PATHS ${TinyXML_PKGCONF_LIBRARY_DIRS})
find_library(TinyXML_LIBRARY_DEBUG NAMES tinyxml${BUILD_SUFIX}gd tinyxml${BUILD_SUFIX}d tinyxml PATHS ${TinyXML_PKGCONF_LIBRARY_DIRS})

if (TinyXML_LIBRARY_RELEASE)
	message("-- Found TinyXML: ${TinyXML_LIBRARY_RELEASE}")
endif ()


# Composite library list (release and debug)
set(TinyXML_LIBRARIES "optimized;${TinyXML_LIBRARY_RELEASE};debug;${TinyXML_LIBRARY_DEBUG}")

if (TinyXML_VERBOSE)
	message("-- Build sufix:" ${CMAKE_BUILD_TYPE} ${BUILD_SUFIX})
	message("-- TinyXML Libraries: ${TinyXML_LIBRARIES}")
endif ()