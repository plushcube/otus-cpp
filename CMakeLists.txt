cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION "0.0.${PATCH_VERSION}")

project("task_01" VERSION ${PROJECT_VESRION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(TASK_CLI "${PROJECT_NAME}_cli")
set(TASK_LIB "version")

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
option(WITH_GOOGLE_TEST "Whether to build Google test" OFF)

add_executable(${TASK_CLI} "src/main.cpp")
add_library(${TASK_LIB} "lib/lib.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)

set_target_properties(${TASK_CLI} ${TASK_LIB} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(${TASK_LIB}
  PUBLIC "include"
)

target_include_directories(${TASK_LIB}
  PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(${TASK_CLI}
  PRIVATE ${TASK_LIB}
)

if (WITH_BOOST_TEST)
  set(TASK_TEST "test_version")

  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  add_executable(${TASK_TEST} "tests/test_version.cpp")

  set_target_properties(${TASK_TEST} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
  )

  set_target_properties(${TASK_TEST} PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
  )

  target_link_libraries(${TASK_TEST}
    ${Boost_LIBRARIES}
    ${TASK_LIB}
  )
endif()

if (WITH_GOOGLE_TEST)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  FetchContent_MakeAvailable(googletest)

  set(TASK_GTEST "gtest_version")
  add_executable(${TASK_GTEST} "tests/gtest_version.cpp")

  target_link_libraries(${TASK_GTEST}
    GTest::gtest_main
    ${TASK_LIB}
  )
endif()

if (MSVC)
  target_compile_options(${TASK_CLI} PRIVATE
    /W4
  )
  target_compile_options(${TASK_LIB} PRIVATE
    /W4
  )
  if(WITH_BOOST_TEST)
    target_compile_options(${TASK_TEST} PRIVATE
      /W4
    )
  endif()
else ()
  target_compile_options(${TASK_CLI} PRIVATE
    -Wall -Wextra -pedantic -Werror
  )
  target_compile_options(${TASK_LIB} PRIVATE
    -Wall -Wextra -pedantic -Werror
  )
  if(WITH_BOOST_TEST)
    target_compile_options(${TASK_TEST} PRIVATE
      -Wall -Wextra -pedantic -Werror
    )
  endif()
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)

if (WITH_BOOST_TEST)
  enable_testing()
  add_test(${TASK_TEST} "test_version")
endif()

if (WITH_GOOGLE_TEST)
  enable_testing()
  include(GoogleTest)
  gtest_discover_tests("gtest_version")
endif()
