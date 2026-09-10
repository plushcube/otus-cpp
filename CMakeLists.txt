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

set(TASK_SERVER "join_server")
set(TASK_CLIENT "join_client")
set(TASK_TEST_STORAGE "test_join_storage")
set(TASK_TEST_SERVER "test_join_server")

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

add_subdirectory(libjoin)

add_executable(${TASK_SERVER} "src/join_server.cpp" "src/utils/utils.cpp")
target_link_libraries(${TASK_SERVER} PRIVATE join::join)

add_executable(${TASK_CLIENT} "src/join_client.cpp" "src/utils/utils.cpp")
target_link_libraries(${TASK_CLIENT} PRIVATE join::join)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

if(MSVC)
  target_compile_options(${TASK_SERVER} PRIVATE /W4)
  target_compile_options(${TASK_CLIENT} PRIVATE /W4)
else()
  target_compile_options(${TASK_SERVER} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${TASK_CLIENT} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_SERVER} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${TASK_SERVER})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)

if(WITH_TESTS)
  add_executable(${TASK_TEST_STORAGE} "tests/test_join_storage.cpp")
  target_link_libraries(${TASK_TEST_STORAGE} PRIVATE GTest::gtest_main
                                                     join::join)

  add_executable(${TASK_TEST_SERVER} "tests/test_join_server.cpp")
  target_link_libraries(${TASK_TEST_SERVER} PRIVATE GTest::gtest_main
                                                    join::join)

  if(MSVC)
    target_compile_options(${TASK_TEST_STORAGE} PRIVATE /W4)
    target_compile_options(${TASK_TEST_SERVER} PRIVATE /W4)
  else()
    target_compile_options(${TASK_TEST_STORAGE} PRIVATE -Wall -Wextra -pedantic
                                                        -Werror)
    target_compile_options(${TASK_TEST_SERVER} PRIVATE -Wall -Wextra -pedantic
                                                       -Werror)
  endif()

  include(GoogleTest)
  gtest_discover_tests(${TASK_TEST_STORAGE})
  gtest_discover_tests(${TASK_TEST_SERVER})
endif()
