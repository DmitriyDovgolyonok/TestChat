cmake_minimum_required(VERSION 3.25)
project(Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(Server main.cpp Session.cpp Session.h Server.cpp Server.h)
find_package(Boost REQUIRED)
target_include_directories(Server PRIVATE ${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(Server ${OPENSSL_LIBRARIES})
