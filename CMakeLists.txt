cmake_minimum_required(VERSION 3.22)
project(Hadik
        VERSION 1.0
        DESCRIPTION "Hra hadik"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(client)
target_sources(client PRIVATE client.c)
target_link_libraries(client PRIVATE ncurses)
target_compile_options(client PRIVATE "-std=c11" "-Wall" "-Wextra" "-Wpedantic")

add_executable(server)
target_sources(server PRIVATE server.c)
target_link_libraries(server PRIVATE ncurses)
target_compile_options(server PRIVATE "-std=c11" "-Wall" "-Wextra" "-Wpedantic")
