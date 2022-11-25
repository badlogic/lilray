cmake_minimum_required(VERSION 3.21.1)
project(lilray)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/emoon/minifb)
FetchContent_MakeAvailable(minifb)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 -D_CRT_SECURE_NO_WARNINGS")

    if (CMAKE_BUILD_TYPE MATCHES Release)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    endif ()

    if (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PROFILE")
    endif ()

    message("MSVC C Flags: ${CMAKE_C_FLAGS}")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-missing-field-initializers")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")

    if (CMAKE_BUILD_TYPE MATCHES Release)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    endif ()
    message("Clang C Flags: ${CMAKE_C_FLAGS}")
    message("Clang C Flags Release: ${CMAKE_C_FLAGS_RELEASE}")
endif ()

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
