cmake_minimum_required(VERSION 3.12)
project(task_manager C)

set(CMAKE_C_STANDARD 11)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})
add_executable(task_manager main.c
        gui/tabs_process.c
        gui/tabs_process.h
        gui/tabs_system.c
        gui/tabs_system.h
        gui/tabs_cpu.c
        gui/tabs_cpu.h
        gui/tabs_memory.c
        gui/tabs_memory.h
        system_info.c
        system_info.h
        process_info.c
        process_info.h
        memory_info.c
        memory_info.h)

target_link_libraries(task_manager ${GTK3_LIBRARIES})