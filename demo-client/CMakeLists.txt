cmake_minimum_required(VERSION 3.10)

project(ServiceRegistryDemoClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(demo-client main.cpp)

target_include_directories(demo-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

