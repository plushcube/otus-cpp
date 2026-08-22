if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.12)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.12)
  project(async VERSION 0.0.1)
endif()

option(WITH_TESTS "Whether to build tests" ON)

add_library(async SHARED "src/async.cpp" "src/parser.cpp" "src/scheduler.cpp")

add_library(async::async ALIAS async)

target_include_directories(
  async PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>)

target_compile_definitions(async PRIVATE ASYNC_BUILDING_LIBRARY)

set_target_properties(async PROPERTIES CXX_VISIBILITY_PRESET hidden
                                       VISIBILITY_INLINES_HIDDEN ON)

target_compile_features(async PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(async PRIVATE /W4)
else()
  target_compile_options(async PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(
  TARGETS async
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

install(FILES include/async/async.h DESTINATION include/async)

if(WITH_TESTS)
  enable_testing()

  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)

  if(MSVC)
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
  endif()

  set(INSTALL_GTEST OFF)
  FetchContent_MakeAvailable(googletest)

  add_executable(test_async tests/test_async.cpp)
  target_link_libraries(test_async PRIVATE GTest::gtest_main async::async)

  if(MSVC)
    target_compile_options(test_async PRIVATE /W4)
  else()
    target_compile_options(test_async PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()

  include(GoogleTest)
  gtest_discover_tests(test_async)
endif()
