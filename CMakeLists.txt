cmake_minimum_required(VERSION 3.10)
project(Calculator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add macOS specific definitions
if(APPLE)
    add_compile_definitions(PTHREAD_SETNAME_TAKES_ONE_ARG)
endif()

# Add Aeron paths
include_directories(
    /usr/local/include
    /usr/local/include/wrapper
    /usr/local/include/aeron
)
link_directories(/usr/local/lib)

# Add executable
add_executable(calculator 
    calculator.cpp
    calculator_functions.h
    calculator_queue.h
)

# Include directories
target_include_directories(calculator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 

# Link against Aeron
target_link_libraries(calculator PRIVATE 
    aeron_client
    aeron_static
) 