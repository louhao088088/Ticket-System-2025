cmake_minimum_required(VERSION 3.22.1)
project(Ticket-System-2025)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Ofast)

add_executable(code 
    src/vector/exceptions.hpp
    src/vector/utility.hpp
    src/vector/vector.hpp
    src/map/exceptions.hpp
    src/map/utility.hpp
    src/map/map.hpp
    src/priority_queue/src/exceptions.hpp
    src/priority_queue/src/utility.hpp
    src/priority_queue/src/priority_queue.hpp
    
    src/MemoryRiver.hpp
    src/Hash.h
    src/BPT.hpp
    src/Operator.h
    src/Ticket.h
    src/Time.h
    src/Train.h
    src/User.h

    src/Hash.cpp
    src/Operator.cpp
    src/Ticket.cpp
    src/Time.cpp
    src/Train.cpp
    src/User.cpp
    main.cpp
)