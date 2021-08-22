cmake_minimum_required(VERSION 3.8)
project(sylar)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -lrt -O3 -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(include/yaml-cpp/include)
# include_directories(sylar)
include_directories(./)

set(LIB_SRC 
    sylar/log.cpp 
    sylar/util.cpp
    sylar/config.cpp
    sylar/thread.cpp
    sylar/mutex.cpp
    sylar/fiber.cpp
    sylar/scheduler.cpp
    sylar/iomanager.cpp
    sylar/timer.cpp
    sylar/hook.cpp
    sylar/fd_manager.cpp
    sylar/address.cpp
    sylar/socket.cpp
    sylar/bytearray.cpp
    sylar/http/http.cpp
    #sylar/http/http11_parser.rl.cpp
    #sylar/http/httpclient_parser.rl.cpp
    sylar/http/http_parser.cpp
    sylar/tcp_server.cpp
    sylar/stream.cpp
    sylar/socket_stream.cpp
    sylar/http/http_session.cpp
    sylar/http/http_server.cpp
    sylar/http/servlet.cpp
    sylar/http/http_connection.cpp
    #sylar/uri.rl.cpp
)

ragelmaker(sylar/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar)

set (LIBS 
    sylar
    dl
    pthread
    #yaml-cpp
    ${PROJECT_SOURCE_DIR}/lib/libyaml-cpp.a
    boost_system
)

#add_library(sylar SHARED ${LIB_SRC})          #动态库
add_library(sylar STATIC ${LIB_SRC})           #静态库
redefine_file_macro(sylar)

sylar_add_executable(test_log "tests/test_log.cc" sylar "${LIBS}")
sylar_add_executable(test_config "tests/test_config.cc" sylar "${LIBS}")
sylar_add_executable(test_thread "tests/test_thread.cc" sylar "${LIBS}")
sylar_add_executable(test_util "tests/test_util.cc" sylar "${LIBS}")
sylar_add_executable(test_fiber "tests/test_fiber.cc" sylar "${LIBS}")
sylar_add_executable(test_scheduler "tests/test_scheduler.cc" sylar "${LIBS}")
sylar_add_executable(test_iomanager "tests/test_iomanager.cc" sylar "${LIBS}")
sylar_add_executable(test_hook "tests/test_hook.cc" sylar "${LIBS}")
sylar_add_executable(test_address "tests/test_address.cc" sylar "${LIBS}")
sylar_add_executable(test_socket "tests/test_socket.cc" sylar "${LIBS}")
sylar_add_executable(test_bytearray "tests/test_bytearray.cc" sylar "${LIBS}")
sylar_add_executable(test_http "tests/test_http.cc" sylar "${LIBS}")
sylar_add_executable(test_http_parser "tests/test_http_parser.cc" sylar "${LIBS}")
sylar_add_executable(test_tcp_server "tests/test_tcp_server.cc" sylar "${LIBS}")
sylar_add_executable(test_http_server "tests/test_http_server.cc" sylar "${LIBS}")
sylar_add_executable(test_http_connection "tests/test_http_connection.cc" sylar "${LIBS}")
sylar_add_executable(test_uri "tests/test_uri.cc" sylar "${LIBS}")

sylar_add_executable(echo_server "examples/echo_server.cc" sylar "${LIBS}")
sylar_add_executable(my_http_server "samples/my_http_server.cc" sylar "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
