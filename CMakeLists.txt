cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(PATCH_VERSION
    "1"
    CACHE INTERNAL "Patch version")
set(PROJECT_VERSION "0.0.${PATCH_VERSION}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("join_server" VERSION ${PROJECT_VERSION})

set(TASK_CLI "join_server")

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
endif()

include(FetchContent)
FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-b2-nodocs.tar.gz)
FetchContent_MakeAvailable(boost)

add_executable(${TASK_CLI} "src/main.cpp" "src/join_server.cpp" "src/session.cpp"
                         "src/join_storage.cpp")
target_include_directories(${TASK_CLI} SYSTEM PRIVATE "${boost_SOURCE_DIR}")

find_package(Threads REQUIRED)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME "join_server")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)

if(WITH_TESTS)
  add_executable(test_join_storage "tests/test_join_storage.cpp" "src/join_storage.cpp")
  target_link_libraries(test_join_storage PRIVATE GTest::gtest_main)
  target_include_directories(test_join_storage PRIVATE "src")

  add_executable(test_join_server "tests/test_join_server.cpp" "src/join_server.cpp"
                                  "src/session.cpp" "src/join_storage.cpp")
  target_link_libraries(test_join_server PRIVATE GTest::gtest_main Threads::Threads)
  target_include_directories(test_join_server PRIVATE "src")
  target_include_directories(test_join_server SYSTEM PRIVATE "${boost_SOURCE_DIR}")

  if(MSVC)
    target_compile_options(test_join_storage PRIVATE /W4)
    target_compile_options(test_join_server PRIVATE /W4)
  else()
    target_compile_options(test_join_storage PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(test_join_server PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()

  include(GoogleTest)
  gtest_discover_tests(test_join_storage)
  gtest_discover_tests(test_join_server)
endif()
