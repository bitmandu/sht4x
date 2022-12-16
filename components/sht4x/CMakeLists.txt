# CMakeLists.txt

set(SRCS src/sht4x.c)
set(REQUIRES driver)

if(${IDF_TARGET} STREQUAL "linux")
  list(APPEND SRCS test/test_sht4x.c)
  list(APPEND REQUIRES unity)
endif()

idf_component_register(
  SRCS ${SRCS}
  INCLUDE_DIRS include
  REQUIRES ${REQUIRES})
