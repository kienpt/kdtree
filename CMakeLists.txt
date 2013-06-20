cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Set DataAccess build type.")
project(preprocess)

add_definitions(-D_FILE_OFSET_BITS=64)

find_path(BOOST_ROOT boost)

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.42 COMPONENTS system iostreams filesystem timer REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES})
