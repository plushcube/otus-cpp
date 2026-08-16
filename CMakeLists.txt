cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION
    "1"
    CACHE INTERNAL "Patch version")
set(PROJECT_VESRION "0.0.${PATCH_VERSION}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("bayan" VERSION ${PROJECT_VESRION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(TASK_CLI "bayan")
set(TASK_LIB "dupfind")

option(WITH_TESTS "Whether to build tests" ON)

add_executable(${TASK_CLI} "src/main.cpp")
add_library(${TASK_LIB} STATIC "lib/config.cpp" "lib/debug.cpp"
                               "lib/dupfind.cpp" "lib/filelist.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

include(FetchContent)
set(BOOST_INCLUDE_LIBRARIES "program_options" "hash2")
FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
  URL_HASH
    SHA256=aca59f889f0f32028ad88ba6764582b63c916ce5f77b31289ad19421a96c555f)
FetchContent_MakeAvailable(boost)

target_include_directories(${TASK_LIB} PUBLIC "include")

target_link_libraries(${TASK_LIB} PUBLIC Boost::program_options Boost::hash2)
target_link_libraries(${TASK_CLI} PRIVATE ${TASK_LIB})

if(WITH_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)
  # For Windows: Prevent overriding the parent project's compiler/linker
  # settings
  if(MSVC)
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
  endif()
  FetchContent_MakeAvailable(googletest)

  set(TASK_TEST "test_lib")
  add_executable(${TASK_TEST} "tests/test_lib.cpp")

  target_link_libraries(${TASK_TEST} GTest::gtest_main ${TASK_LIB})
endif()

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
  target_compile_options(${TASK_LIB} INTERFACE /W4)
  if(WITH_TESTS)
    target_compile_options(${TASK_TEST} PRIVATE /W4)
  endif()
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${TASK_LIB} INTERFACE -Wall -Wextra -pedantic -Werror)
  if(WITH_TESTS)
    target_compile_options(${TASK_TEST} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)

if(WITH_TESTS)
  enable_testing()
  include(GoogleTest)
  gtest_discover_tests(${TASK_TEST})
endif()
