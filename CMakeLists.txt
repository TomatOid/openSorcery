project(openSourcery)

include_directories(include libs/enet/include)
find_file(SDL2_INCLUDE_DIR NAME SDL.h HINTS SDL2)
find_file(SDL2_INCLUDE_DIR NAME SDL_timer.h HINTS SDL2)
find_library(SDL2_LIBRARY NAME SDL2)

add_subdirectory(libs/enet)

set(SERVER_SOURCES src/server.c src/generated_serialize.c)

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server enet SDL2 pthread)
set(CLIENT_SOURCES src/client.c src/generated_serialize.c)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client enet SDL2 pthread)
