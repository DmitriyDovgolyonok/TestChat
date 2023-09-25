cmake_minimum_required(VERSION 3.25)
project(Client)

set(CMAKE_CXX_STANDARD 17)

add_executable(Client main.cpp Client.cpp Client.h)
find_package(Boost REQUIRED)
target_include_directories(Client PRIVATE ${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(Client ${OPENSSL_LIBRARIES})
