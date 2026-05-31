cmake_minimum_required(VERSION 3.10)
project(ServiceRegistry)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# ========== 注册中心服务端 ==========
add_executable(registry-server
    src/main.cpp
    src/registry.cpp
)

target_include_directories(registry-server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${THIRD_PARTY_DIR}
)

if(WIN32)
    target_link_libraries(registry-server PRIVATE ws2_32)
else()
    target_link_libraries(registry-server PRIVATE pthread)
endif()

# ========== 示例微服务客户端 ==========
add_executable(demo-client
    demo-client/main.cpp
)

target_include_directories(demo-client PRIVATE
    ${THIRD_PARTY_DIR}
)

if(WIN32)
    target_link_libraries(demo-client PRIVATE ws2_32)
else()
    target_link_libraries(demo-client PRIVATE pthread)
endif()
