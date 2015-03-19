cmake_minimum_required(VERSION 2.6)
project(fbox)


# Uncomment to enable FBox logging of debug messages 
#add_definitions(-DFBOX_DEBUG)

# Uncomment to enable multithreaded build
set(FBOX_MULTITHREADED "ON")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include (FBoxHelperMacros)

	

#######################################################
# Libraries and includes
#######################################################

include_directories(${CMAKE_INSTALL_PREFIX}/include) # Add install directory to include search path

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIR})

find_package(CURL 7.18.0 REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

# Uncomment if using libcurl static library
add_definitions(-DCURL_STATICLIB)

message ("-- libcurl includes: ${CURL_INCLUDE_DIR}")
message ("-- libcurl libraries: ${CURL_LIBRARIES}")


if (WIN32)
	set(PLATFORM_LIBRARIES ws2_32 wldap32 iphlpapi winhttp)
elseif(UNIX)
	set(PLATFORM_LIBRARIES)
endif ()



#######################################################
# Load Boost dependencies
#######################################################

#set(Boost_DEBUG 1)

if (FBOX_MULTITHREADED)
	set(Boost_USE_MULTITHREADED ON)
else ()
	set(Boost_USE_MULTITHREADED OFF)
endif ()

if (FBOX_STATIC_RUNTIME)
	set(Boost_USE_STATIC_RUNTIME ON)
else ()
	set(Boost_USE_STATIC_RUNTIME OFF)
endif()


# Use Boost static libs
set(Boost_USE_STATIC_LIBS ON)
add_definitions(-DBOOST_PYTHON_STATIC_LIB)

# Enable if setting Boost_USE_STATIC_LIBS OFF
#add_definitions(-DBOOST_TEST_DYN_LINK) 

# Disable autolinking in Windows MSVC
add_definitions(-DBOOST_ALL_NO_LIB)

if (EXISTS "../../../contrib/include/boost")
	set(BOOST_ROOT ../../contrib)
	message("-- Setting BOOST_ROOT: ${BOOST_ROOT}")
endif ()

find_package(Boost 1.53 COMPONENTS system filesystem date_time program_options unit_test_framework REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# commonly used libraries throughout the project
#set(BOOST_COMMON_LIBRARIES ${Boost_DATE_TIME_LIBRARIES} ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})
#message ("-- Boost common libraries: ${BOOST_COMMON_LIBRARIES}")
#message ("-- Boost test library: ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
#message ("-- Boost program options library: ${Boost_PROGRAM_OPTIONS_LIBRARY}")




#######################################################
# Tool settings
#
# CURL_STATICLIB            Link statically with libcurl
# TIXML_USE_STL             TinyXML uses STL strings and streams
#
#######################################################

add_definitions(-DTIXML_USE_STL)

#set(CMAKE_VERBOSE_MAKEFILE)

if(MSVC)
  # StringPooling: true == /GF  false == /GF-
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GF")

  # RuntimeLibrary
  # 0 (MultiThreaded) == /MT
  # 1 (MultiThreadedDebug) == /MTd
  # 2 (MultiThreadedDLL) == /MD
  # 3 (MultiThreadedDebugDLL) == /MDd
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

  # TreatWChar_tAsBuiltInType: true == /Zc:wchar_t  false == /Zc:wchar_t-
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t")

  # WarningLevel: /W<level 0 to 4> or /Wall
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")

else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


# Append postfix 'd' to debug builds
set(CMAKE_DEBUG_POSTFIX "d") 



#######################################################
# Sub-directories
#######################################################


add_subdirectory(core)
add_subdirectory(simulate)
