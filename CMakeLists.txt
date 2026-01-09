cmake_minimum_required(VERSION 3.10)
project(android-so-injector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Android specific settings
if(ANDROID)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/include
)

# Source files
set(INJECTOR_SOURCES
    src/main.cpp
    src/injector.cpp
    src/ptrace_utils.cpp
    src/process_utils.cpp
    src/elf_utils.cpp
)

# Build injector executable
add_executable(injector ${INJECTOR_SOURCES})

# Link libraries
target_link_libraries(injector
    log
    dl
)

# Example injectable library
add_library(example_lib SHARED
    src/example_lib.cpp
)

target_link_libraries(example_lib
    log
)

# Install targets
install(TARGETS injector DESTINATION bin)
install(TARGETS example_lib DESTINATION lib)
