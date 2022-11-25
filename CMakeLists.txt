cmake_minimum_required(VERSION 3.21.1)
project(lilray)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/emoon/minifb)
FetchContent_MakeAvailable(minifb)

include_directories(src)
add_executable(lilray "src/lilray.cpp" "src/main.cpp")
target_link_libraries(lilray LINK_PUBLIC minifb)
add_custom_command(
        TARGET lilray POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/wolftextures.png
        ${CMAKE_CURRENT_BINARY_DIR}/wolftextures.png)

add_executable(lilray_c "src/lilray.cpp" "src/lilray-c.cpp"  "src/main.c")
target_link_libraries(lilray_c LINK_PUBLIC minifb)
add_custom_command(
        TARGET lilray_c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/wolftextures.png
        ${CMAKE_CURRENT_BINARY_DIR}/wolftextures.png)
