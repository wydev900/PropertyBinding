set(TestName Tests)
project(${TestName} LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DGTEST_LANGUAGE_CXX20)


include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

if(BUILD_GMOCK)
    include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
endif()

file(GLOB_RECURSE SOURCEFILES "*.h" "*.cpp")

add_executable(${TestName} ${SOURCEFILES})

target_link_libraries(${TestName} gtest gtest_main CppUI)
# set_property(TARGET ${TestName} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_test(NAME ${TestName} COMMAND ${TestName})