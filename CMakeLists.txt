cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(PATCH_VERSION "1" CACHE STRING "Patch version")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("mlp" VERSION 0.0.${PATCH_VERSION})

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

add_subdirectory(libmlp)

set(TASK_CLI "fashio_mnist")

add_executable(${TASK_CLI} "src/main.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

target_link_libraries(${TASK_CLI} PRIVATE mnist::mlp)
target_include_directories(${TASK_CLI} PRIVATE "${PROJECT_BINARY_DIR}")

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)

option(WITH_TESTS "Whether to build tests" ON)

if(WITH_TESTS)
  enable_testing()

  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)

  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()

  set(INSTALL_GTEST OFF)
  FetchContent_MakeAvailable(googletest)

  include(GoogleTest)

  set(TEST_TASK "test_mlp")

  add_executable(${TEST_TASK} "tests/test_mlp.cpp")

  # target_compile_definitions(
  #   ${TASK_TEST_PIPELINE}
  #   PRIVATE MR_DATASET_PATH="${PROJECT_SOURCE_DIR}/input/AB_NYC_2019.csv")

    target_link_libraries(${TEST_TASK} PRIVATE GTest::gtest_main mnist::mlp)

    if(MSVC)
      target_compile_options(${TEST_TASK} PRIVATE /W4)
    else()
      target_compile_options(${TEST_TASK} PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()

    gtest_discover_tests(${TEST_TASK})
endif()

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)
