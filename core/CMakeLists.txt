file(GLOB SOURCES *.cpp *.h)
add_library(core ${SOURCES})

install(TARGETS core 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

fbox_install_headers(core fbox)

add_subdirectory(test)
