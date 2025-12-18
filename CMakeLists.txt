cmake_minimum_required(VERSION 3.16)

project(TCPServer_Design LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== Build options (빌드 스크립트에서 -D로 제어) =====
option(BUILD_SERVER "Build tcp_server executable" ON)
option(BUILD_SERVER_LIBS "Build server libraries (static/shared)" ON)
option(BUILD_CLIENT "Build test client" ON)

# 공용 include (nlohmann json single header)
# (네 기존 설정 유지)
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib")

# 빌드 타입이 지정되지 않았으면 Debug로
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Options: BUILD_SERVER=${BUILD_SERVER}, BUILD_SERVER_LIBS=${BUILD_SERVER_LIBS}, BUILD_CLIENT=${BUILD_CLIENT}")

# ===== Subdirectories =====
if (BUILD_SERVER OR BUILD_SERVER_LIBS)
    add_subdirectory(Server)
endif()

if(BUILD_CLIENT)
    add_subdirectory(Client)
endif()
