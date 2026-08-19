cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION
    "1"
    CACHE INTERNAL "Patch version")
set(PROJECT_VERSION "0.0.${PATCH_VERSION}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("bayan" VERSION ${PROJECT_VERSION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(TASK_CLI "bayan")
set(TASK_LIB "dupfind")

option(WITH_TESTS "Whether to build tests" ON)

add_executable(${TASK_CLI} "src/main.cpp")
add_library(
  ${TASK_LIB} STATIC "lib/config.cpp" "lib/debug.cpp" "lib/dupfind.cpp"
                     "lib/filelist.cpp" "lib/hasher.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

include(FetchContent)
set(BOOST_INCLUDE_LIBRARIES "program_options" "hash2" "crc")
FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
  URL_HASH
    SHA256=aca59f889f0f32028ad88ba6764582b63c916ce5f77b31289ad19421a96c555f)
FetchContent_MakeAvailable(boost)

target_include_directories(${TASK_LIB} PUBLIC "include")

target_link_libraries(${TASK_LIB} PUBLIC Boost::program_options Boost::hash2 Boost::crc)
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

  add_library(test_utils STATIC "tests/utils/utils.cpp")
  target_include_directories(test_utils PUBLIC "tests")
  target_link_libraries(test_utils PUBLIC ${TASK_LIB})
  target_compile_definitions(
    test_utils
    PUBLIC "FIXTURES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures\"")

  set(TASK_TESTS test_filelist test_config test_dupfind)
  foreach(t ${TASK_TESTS})
    add_executable(${t} "tests/${t}.cpp")
    target_link_libraries(${t} GTest::gtest_main test_utils ${TASK_LIB})
  endforeach()
endif()

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
  target_compile_options(${TASK_LIB} INTERFACE /W4)
  if(WITH_TESTS)
    target_compile_options(test_utils PRIVATE /W4)
    foreach(t ${TASK_TESTS})
      target_compile_options(${t} PRIVATE /W4)
    endforeach()
  endif()
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${TASK_LIB} INTERFACE -Wall -Wextra -pedantic -Werror)
  if(WITH_TESTS)
    target_compile_options(test_utils PRIVATE -Wall -Wextra -pedantic -Werror)
    foreach(t ${TASK_TESTS})
      target_compile_options(${t} PRIVATE -Wall -Wextra -pedantic -Werror)
    endforeach()
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
  foreach(t ${TASK_TESTS})
    gtest_discover_tests(${t})
  endforeach()
endif()
