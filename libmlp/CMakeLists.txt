if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.12)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.12)
  project(mlp VERSION 0.0.1)
endif()

add_library(mlp STATIC "src/mlp_classifier.cpp")
add_library(mnist::mlp ALIAS mlp)

include(FetchContent)
FetchContent_Declare(
  eigen
  URL https://gitlab.com/libeigen/eigen/-/archive/5.0.1/eigen-5.0.1.tar.bz2
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(eigen)

target_include_directories(mlp PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
# PUBLIC: <mnist/mlp_classifier.h> includes <Eigen/Dense>, so consumers need Eigen too.
target_link_libraries(mlp PUBLIC Eigen3::Eigen)

if(MSVC)
  target_compile_options(mlp PRIVATE /W4)
else()
  target_compile_options(mlp PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
