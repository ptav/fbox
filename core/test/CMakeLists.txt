file(GLOB SOURCES *.cpp)

add_executable(test_core ${SOURCES})

target_link_libraries(test_core
  core
  tinyxml
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARIES}
  ${Boost_SYSTEM_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

install(TARGETS test_core RUNTIME DESTINATION test/fbox)

