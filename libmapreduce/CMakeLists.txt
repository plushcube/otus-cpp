if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.12)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.12)
  project(mapreduce VERSION 0.0.1)
endif()

add_library(mapreduce STATIC "src/map_reduce.cpp")

add_library(mapreduce::mapreduce ALIAS mapreduce)

target_include_directories(
  mapreduce PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(MSVC)
  target_compile_options(mapreduce PRIVATE /W4)
else()
  target_compile_options(mapreduce PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
