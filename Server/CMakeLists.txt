cmake_minimum_required(VERSION 3.16)

# 상위 옵션 사용
option(BUILD_SERVER "Build tcp_server executable" ON)
option(BUILD_SERVER_LIBS "Build server libraries (static/shared)" ON)

# 공통 소스
set(CPP_SOURCES
    TcpServer.cpp
    ExampleMessageHandler.cpp
)

set(CPP_HEADERS
    Dispatcher.h
    Message.h
    ThreadSafeQueue.h
    ExampleMessageHandler.h
    TcpServer.h
)

# 공통 include
set(INCLUDES
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 공통 컴파일 옵션/정의 적용 함수
function(apply_common_settings tgt)
    target_include_directories(${tgt} PRIVATE ${INCLUDES})

    target_compile_options(${tgt} PRIVATE
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
        $<$<CONFIG:Release>:-O3>
    )

    target_compile_definitions(${tgt} PRIVATE
        $<$<CONFIG:Debug>:DEBUG_BUILD>
        $<$<CONFIG:Release>:NDEBUG>
    )

    find_package(Threads REQUIRED)
    target_link_libraries(${tgt} PRIVATE Threads::Threads)
endfunction()

# ===== Libraries (static + shared) =====
if(BUILD_SERVER_LIBS)
    # so 만들 때 PIC 필요(정적도 함께 켜도 무해)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    add_library(tcpserver_static STATIC ${CPP_SOURCES} ${C_SOURCES} ${CPP_HEADERS})
    set_target_properties(tcpserver_static PROPERTIES OUTPUT_NAME tcpserver)
    apply_common_settings(tcpserver_static)

    add_library(tcpserver_shared SHARED ${CPP_SOURCES} ${C_SOURCES} ${CPP_HEADERS})
    set_target_properties(tcpserver_shared PROPERTIES OUTPUT_NAME tcpserver)
    apply_common_settings(tcpserver_shared)
endif()

# ===== Executable =====
if(BUILD_SERVER)
    add_executable(tcp_server main.cpp)
    target_sources(tcp_server PRIVATE ${CPP_SOURCES} ${C_SOURCES} ${CPP_HEADERS})
    apply_common_settings(tcp_server)

    # (선택) 실행파일이 라이브러리를 링크하게 바꾸고 싶으면 아래처럼:
    # if(BUILD_SERVER_LIBS)
    #   target_link_libraries(tcp_server PRIVATE tcpserver_static)
    # endif()
endif()
