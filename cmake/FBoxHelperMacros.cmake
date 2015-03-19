# Copy header files in the local directory to ${CMAKE_INSTALL_PREFIX}/${DESTINATION}.
# Make command is a dependency of ${TARGET}
#
macro(fbox_install_headers TARGET DESTINATION)
    message("Installing ${TARGET} headers to ${DESTINATION}")

	add_custom_command(TARGET ${TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include/${DESTINATION} COMMENT "Create folder ${CMAKE_INSTALL_PREFIX}/include/${DESTINATION}" )
		
	file(GLOB HEADERS *.h)
	
	foreach(FILE ${HEADERS})
		add_custom_command(TARGET ${TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${FILE} ${CMAKE_INSTALL_PREFIX}/include/${DESTINATION} COMMENT "Copy ${HEADER} to ${CMAKE_INSTALL_PREFIX}/include/${DESTINATION}" )
	endforeach()

endmacro()


# Set FBOX_BUILD_SUFIX depeding on platform and build settings
#
# Set the following variables:
#	FBOX_MULTITHREADED    Set if building multithreaded
#	FBOX_STATIC_RUNTIME   Set if static linking against runtime libraries
#	FBOX_DEBUG_RUNTIME	  Set if linking against debug runtime libraries
#
# Platform:
#	mgw48, vc100, vc110
#
# Build settings:
#	m	Multithreaded
#	s	Static runtime
#	d	Include debug information
#	g	Lik to debug runtime
#
macro(fbox_build_sufix VAR)
	set(${VAR} "")
	
	if (MINGW)
		set (TOOLSET "mgw48")
	elseif (MSVC10)
		set (TOOLSET "vc100")
	elseif (MSVC11)
		set (TOOLSET "vc110")
	endif ()

	if (FBOX_MULTITHREADED)
		set (BUILD "m")
	endif ()

	if (FBOX_STATIC_RUNTIME)
		set (BUILD "${BUILD}s")
	endif()

	if (FBOX_DEBUG_RUNTIME)
		set (BUILD "${BUILD}g")
	endif()

	if (CMAKE_BUILD_TYPE MATCHES "Debug")
		set (BUILD "${BUILD}d")
	endif()

	if (BUILD)
		set (BUILD "-${BUILD}")
	endif ()

	if (TOOLSET)
		set (TOOLSET "-${TOOLSET}")
	endif ()
	
	set(${VAR} "${TOOLSET}${BUILD}")

endmacro()
