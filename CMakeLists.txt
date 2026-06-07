cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION "0.0.${PATCH_VERSION}")

project("task_03" VERSION ${PROJECT_VESRION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(TASK_CLI "task_03")
set(TASK_LIB "libtask03")

option(WITH_TESTS "Whether to build tests" ON)

add_executable(${TASK_CLI} "src/main.cpp")
add_library(${TASK_LIB} "lib/empty.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)

set_target_properties(${TASK_CLI} ${TASK_LIB} PROPERTIES
  CXX_STANDARD 20
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

if (WITH_TESTS)
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

  set(TASK_TEST "test_container")
  add_executable(${TASK_TEST}
    "tests/test_allocator.cpp"
    "tests/test_container.cpp"
  )

  target_link_libraries(${TASK_TEST}
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
  if(WITH_TESTS)
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
  if(WITH_TESTS)
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

if (WITH_TESTS)
  enable_testing()
  include(GoogleTest)
  gtest_discover_tests(${TASK_TEST})
endif()
