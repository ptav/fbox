file(GLOB SOURCES *.cpp *.h)
add_library(simulate ${SOURCES})
target_link_libraries(simulate core ${LIBRARIES})

install(TARGETS simulate 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

fbox_install_headers(simulate fbox/simulate)
	
add_subdirectory(test)