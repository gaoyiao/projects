cmake_minimum_required(VERSION 3.24)

project(client)

set(CMAKE_CXX_STANDARD 20)
if(APPLE)
    set(CMAKE_INSTALL_RPATH "/usr/local/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif ()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#add_executable(client main.cpp
#                      client.cpp
#                      utils.cpp
#                      data.cpp)

add_executable(server main.cpp
                      server.cpp
                      utils.cpp
                      data.cpp)

target_link_libraries(server ${OpenCV_LIBS})
