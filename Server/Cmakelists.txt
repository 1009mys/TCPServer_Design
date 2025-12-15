cmake_minimum_required(VERSION 3.16)

add_executable(tcp_server
    main.cpp
)

target_sources(tcp_server
    PRIVATE
        TcpServer.cpp
        Message.h
        ThreadSafeQueue.h
)

target_include_directories(tcp_server
    PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(tcp_server PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
    $<$<CONFIG:Release>:-O3>
)

target_compile_definitions(tcp_server PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:NDEBUG>
)


# Linux: pthread 필요
find_package(Threads REQUIRED)
target_link_libraries(tcp_server PRIVATE Threads::Threads)
