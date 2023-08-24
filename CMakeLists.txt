cmake_minimum_required(VERSION 3.24)
project(ftp_server)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-std=c++0x")
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/)
#[[add_executable(ftp_server main.cpp ftpserver.cpp ftpserver.h consts.h)]]
add_executable(ftp_server main.cpp ftpserver.cpp ftpserver.h display.cpp display.h consts.h)

if(WIN32)
    #[[target_link_libraries(ftp_server pdcurses.lib)]]
    target_link_libraries(ftp_server wsock32 ws2_32 pdcurses.lib)
endif()
set(CMAKE_BUILD_TYPE Debug)
