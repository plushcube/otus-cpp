cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

set(PATCH_VERSION "1" CACHE STRING "Patch version")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("map_reduce" VERSION 0.0.${PATCH_VERSION})

set(TASK_MAPPER "mapper")
set(TASK_REDUCER "reducer")

add_executable(${TASK_MAPPER} "src/mapper.cpp")
add_executable(${TASK_REDUCER} "src/reducer.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

target_include_directories(${TASK_MAPPER} PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(${TASK_REDUCER} PRIVATE "${PROJECT_BINARY_DIR}")

if(MSVC)
  target_compile_options(${TASK_MAPPER} PRIVATE /W4)
  target_compile_options(${TASK_REDUCER} PRIVATE /W4)
else()
  target_compile_options(${TASK_MAPPER} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${TASK_REDUCER} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_MAPPER} RUNTIME DESTINATION bin)
install(TARGETS ${TASK_REDUCER} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)

