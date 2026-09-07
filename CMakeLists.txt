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

project("bulk" VERSION ${PROJECT_VERSION})

set(TASK_CLI "bulk_server")

option(WITH_TESTS "Whether to build tests" ON)

# Должен вызываться до add_subdirectory, иначе поддиректории обрабатываются с
# testing=OFF и их тесты не регистрируются в ctest
if(WITH_TESTS)
  enable_testing()

  include(FetchContent)

  # googletest подключается на уровне корня: так его таргеты видны и тестам
  # libasync, и тестам bulk_server (задание 10).
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

# Boost.Asio для bulk_server (задание 10) подключается через FetchContent,
# чтобы сборка не зависела от системных установок Boost. В исходном архиве
# Boost нет корневого CMakeLists.txt, поэтому MakeAvailable только распакует
# его; таргеты получат include-путь ${boost_SOURCE_DIR} (Asio header-only,
# линковка не нужна). URL_HASH можно зафиксировать позже по SHA256 архива.
include(FetchContent)
FetchContent_Declare(
  boost
  URL https://archives.boost.io/release/1.92.0/source/boost_1_92_0.tar.gz)
FetchContent_MakeAvailable(boost)

add_subdirectory(libasync)

add_executable(${TASK_CLI} "src/main.cpp")
target_link_libraries(${TASK_CLI} PRIVATE async::async)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME "bulk_server")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)
