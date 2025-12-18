cmake_minimum_required(VERSION 3.16)

project(TCPServer_Design LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 공용 include (nlohmann json single header)
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib")

# 빌드 타입이 지정되지 않았으면 Debug로
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")



add_subdirectory(Server)
add_subdirectory(Client)
