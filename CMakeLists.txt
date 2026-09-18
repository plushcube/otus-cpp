cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(PATCH_VERSION "1" CACHE STRING "Patch version")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("map_reduce" VERSION 0.0.${PATCH_VERSION})

# По умолчанию CMake собирает без оптимизаций: для mapper/reducer это
# четырёхкратная разница на ровном месте.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release
                                                  RelWithDebInfo MinSizeRel)
endif()

add_subdirectory(libmapreduce)

set(TASK_MAPPER "mapper")
set(TASK_REDUCER_MEAN "reducer_mean")
set(TASK_REDUCER_VARIANCE "reducer_variance")

add_executable(${TASK_MAPPER} "src/mapper.cpp")
add_executable(${TASK_REDUCER_MEAN} "src/reducer_mean.cpp")
add_executable(${TASK_REDUCER_VARIANCE} "src/reducer_variance.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

set(TASKS ${TASK_MAPPER} ${TASK_REDUCER_MEAN} ${TASK_REDUCER_VARIANCE})

foreach(TASK IN LISTS TASKS)
  target_link_libraries(${TASK} PRIVATE mapreduce::mapreduce)
  target_include_directories(${TASK} PRIVATE "${PROJECT_BINARY_DIR}")

  if(MSVC)
    target_compile_options(${TASK} PRIVATE /W4)
  else()
    target_compile_options(${TASK} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()

  install(TARGETS ${TASK} RUNTIME DESTINATION bin)
endforeach()

option(WITH_TESTS "Whether to build tests" ON)

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

  include(GoogleTest)

  set(TASK_TEST_MAPPER "test_mapper")
  set(TASK_TEST_REDUCER_MEAN "test_reducer_mean")
  set(TASK_TEST_REDUCER_VARIANCE "test_reducer_variance")
  set(TASK_TEST_PIPELINE "test_pipeline")

  add_executable(${TASK_TEST_MAPPER} "tests/test_mapper.cpp")
  add_executable(${TASK_TEST_REDUCER_MEAN} "tests/test_reducer_mean.cpp")
  add_executable(${TASK_TEST_REDUCER_VARIANCE} "tests/test_reducer_variance.cpp")
  add_executable(${TASK_TEST_PIPELINE} "tests/test_pipeline.cpp")

  target_compile_definitions(
    ${TASK_TEST_PIPELINE}
    PRIVATE MR_DATASET_PATH="${PROJECT_SOURCE_DIR}/input/AB_NYC_2019.csv")

  set(TEST_TASKS ${TASK_TEST_MAPPER} ${TASK_TEST_REDUCER_MEAN}
                 ${TASK_TEST_REDUCER_VARIANCE} ${TASK_TEST_PIPELINE})

  foreach(TEST_TASK IN LISTS TEST_TASKS)
    target_link_libraries(${TEST_TASK} PRIVATE GTest::gtest_main
                                               mapreduce::mapreduce)

    if(MSVC)
      target_compile_options(${TEST_TASK} PRIVATE /W4)
    else()
      target_compile_options(${TEST_TASK} PRIVATE -Wall -Wextra -pedantic
                                                  -Werror)
    endif()

    gtest_discover_tests(${TEST_TASK})
  endforeach()
endif()

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)
