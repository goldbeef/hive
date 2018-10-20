cmake_minimum_required(VERSION 3.1)
project(hive)
aux_source_directory(. hive_src)
set(CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" SUPPORT_CXX11)
if (NOT SUPPORT_CXX11)
	message(FATAL_ERROR "compiler not support c++11")
endif()

find_package(luna REQUIRED)
find_package(Threads REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
endif()

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" )
    include_directories("/usr/local/include")
    link_directories("/usr/local/lib")
endif()

add_executable(hive ${hive_src})
target_link_libraries(hive luna lua dl Threads::Threads)

install(TARGETS hive RUNTIME DESTINATION bin)

