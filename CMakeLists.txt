cmake_minimum_required(VERSION 3.22.1)
project(Ticket-System-2025)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Ofast)

add_executable(code 
    src/exception.hpp
    src/utility.hpp
    src/vector.hpp
    src/BPT.hpp
    main.cpp
)