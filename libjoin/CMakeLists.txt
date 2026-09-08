if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.12)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.12)
  project(join VERSION 0.0.1)
endif()

add_library(join STATIC "src/join_storage.cpp" "src/session.cpp"
                        "src/join_server.cpp" "src/command_executor.cpp")

add_library(join::join ALIAS join)

include(FetchContent)
FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-b2-nodocs.tar.gz
)
FetchContent_MakeAvailable(boost)

find_package(Threads REQUIRED)
target_link_libraries(join PUBLIC Threads::Threads)

target_include_directories(
  join PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(join SYSTEM PUBLIC "${boost_SOURCE_DIR}")

if(MSVC)
  target_compile_options(join PRIVATE /W4)
else()
  target_compile_options(join PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
