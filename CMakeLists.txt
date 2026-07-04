cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION "0.0.${PATCH_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("editor" VERSION ${PROJECT_VESRION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(TASK_CLI "editor_app")
set(TASK_LIB "editor")

option(WITH_TESTS "Whether to build tests" ON)

add_executable(${TASK_CLI} "src/main.cpp")
add_library(${TASK_LIB} STATIC)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)

target_include_directories(${TASK_LIB}
  PUBLIC "include"
)

target_sources(${TASK_LIB} PRIVATE
  "lib/controllers/canvas.cpp"
  "lib/controllers/editor.cpp"
  "lib/models/image.cpp"
  "lib/services/svg_service.cpp"
  "lib/shapes/circle.cpp"
  "lib/shapes/ellipse.cpp"
  "lib/shapes/line.cpp"
  "lib/shapes/rectangle.cpp"
  "lib/ui/editor_view.cpp"
)

target_link_libraries(${TASK_CLI}
  PRIVATE ${TASK_LIB}
)

if (WITH_TESTS)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  FetchContent_MakeAvailable(googletest)

  set(TASK_TEST "test_lib")
  add_executable(${TASK_TEST} "tests/test_lib.cpp")

  target_link_libraries(${TASK_TEST}
    GTest::gtest_main
    ${TASK_LIB}
  )
endif()

if (MSVC)
  target_compile_options(${TASK_CLI} PRIVATE
    /W4
  )
  target_compile_options(${TASK_LIB} INTERFACE
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
  target_compile_options(${TASK_LIB} INTERFACE
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
