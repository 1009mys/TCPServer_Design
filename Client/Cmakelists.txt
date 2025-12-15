cmake_minimum_required(VERSION 3.16)

add_executable(tcp_client
    TcpClient.cpp
)

target_include_directories(tcp_client
    PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(tcp_client PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
    $<$<CONFIG:Release>:-O3>
)

target_compile_definitions(tcp_client PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:NDEBUG>
)
