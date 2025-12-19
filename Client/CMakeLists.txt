cmake_minimum_required(VERSION 3.16)

add_executable(tcp_client
    TcpClient.cpp
)

# (선택) tcp_client가 서버 라이브러리를 링크해야 하면 ON
option(CLIENT_LINK_SERVER_LIB "Link tcp_client with server library" OFF)

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


find_package(Threads REQUIRED)
target_link_libraries(tcp_client PRIVATE Threads::Threads)

# (선택) 서버 라이브러리와 링크
# - Server/CMakeLists.txt에서 tcpserver_static/tcpserver_shared 타겟이 생성되는 경우만 가능
if(CLIENT_LINK_SERVER_LIB)
    if(TARGET tcpserver_static)
        target_link_libraries(tcp_client PRIVATE tcpserver_static)
    elseif(TARGET tcpserver_shared)
        target_link_libraries(tcp_client PRIVATE tcpserver_shared)
    else()
        message(FATAL_ERROR "CLIENT_LINK_SERVER_LIB=ON but no server library target exists. Enable BUILD_SERVER_LIBS.")
    endif()
endif()