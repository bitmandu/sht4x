# CMakeLists.txt

cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(COMPONENTS main)
set(TEST_TARGET esp32s2)

project(sht4x_test)
idf_build_set_property(COMPILE_OPTIONS "-DCONFIG_IDF_TARGET_ESP32S2" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wall" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wextra" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-fanalyzer" APPEND)
