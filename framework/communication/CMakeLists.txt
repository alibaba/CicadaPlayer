cmake_minimum_required(VERSION 3.6)
project(communication)
set(CMAKE_CXX_STANDARD 11)
add_library(communication ${TARGET_LIBRARY_TYPE} "")
target_sources(communication PRIVATE
        messageServer.cpp
        messageServer.h
        IProtocol.cpp
        IProtocol.h
        TCPProtocol.cpp
        TCPProtocol.h)
target_include_directories(communication PRIVATE
        ${COMMON_INC_DIR}
        )