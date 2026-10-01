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

add_executable(${TASK_CLI} "src/main.cpp" "src/arg_parser.cpp" "src/runners/mlp_runner.cpp")

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")

target_link_libraries(${TASK_CLI} PRIVATE mnist::mlp)
target_include_directories(${TASK_CLI} PRIVATE "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Веса модели ищутся рядом с исполняемым файлом — кладём их туда при сборке
# и установке, чтобы бинарник был самодостаточным и не зависел от CWD.
set(MODEL_WEIGHTS
    "${CMAKE_CURRENT_SOURCE_DIR}/input/w1.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/input/w2.txt")

add_custom_command(
  TARGET ${TASK_CLI}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MODEL_WEIGHTS} "$<TARGET_FILE_DIR:${TASK_CLI}>"
  COMMENT "Копирую веса модели рядом с ${TASK_CLI}")

if(MSVC)
  target_compile_options(${TASK_CLI} PRIVATE /W4)
else()
  target_compile_options(${TASK_CLI} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${TASK_CLI} RUNTIME DESTINATION bin)
install(FILES ${MODEL_WEIGHTS} DESTINATION bin)

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

  add_executable(${TEST_TASK} "tests/test_mlp.cpp" "tests/helpers.cpp")

  target_include_directories(${TEST_TASK} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")

  target_link_libraries(${TEST_TASK} PRIVATE GTest::gtest_main mnist::mlp)

  # Тесты открывают данные относительными путями вида "train/w1.txt", поэтому
  # запускаем их с рабочим каталогом рядом с бинарником и кладём данные туда же —
  # та же схема, что и у весов модели в fashio_mnist.
  #
  # Веса и эталонные вероятности — согласованный набор: probas в test_mnist_mlp.txt
  # посчитаны именно на tests/data/train/w*.txt (они отличаются от input/w*.txt,
  # которые использует fashio_mnist). Менять их по отдельности нельзя.
  set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/train")

  if(NOT EXISTS "${TEST_DATA_DIR}")
    message(FATAL_ERROR "TEST_DATA_DIR не найден: ${TEST_DATA_DIR} — тесты не смогут прочитать train/…")
  endif()

  add_custom_command(
    TARGET ${TEST_TASK}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_DATA_DIR}"
            "$<TARGET_FILE_DIR:${TEST_TASK}>/train"
    COMMENT "Копирую данные для тестов рядом с ${TEST_TASK}")

  if(MSVC)
    target_compile_options(${TEST_TASK} PRIVATE /W4)
  else()
    target_compile_options(${TEST_TASK} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()

  gtest_discover_tests(${TEST_TASK} WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TEST_TASK}>")
endif()

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "https://t.me/plushcube")
include(CPack)
