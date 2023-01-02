cmake_minimum_required(VERSION 3.21.1)
project(lilray)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    set(CMAKE_OSX_ARCHITECTURES x86_64)
#    set(ONLY_ACTIVE_ARCH NO)
#endif()

set(CMAKE_CXX_FLAGS -DMINIFB_AVOID_CPP_HEADERS)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/badlogic/minifb GIT_TAG dos-pr)
set(MINIFB_BUILD_EXAMPLES CACHE INTERNAL FALSE)
FetchContent_MakeAvailable(minifb)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=undefined")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")

include_directories(src)

add_executable(lilray "src/lilray.cpp" "src/main.cpp")
target_link_libraries(lilray LINK_PUBLIC minifb)

add_executable(lilray_c "src/lilray.cpp" "src/lilray-c.cpp"  "src/main.c")
target_link_libraries(lilray_c LINK_PUBLIC minifb)

add_executable(subpixel "src/subpixel.cpp" "src/lilray.cpp")
target_link_libraries(subpixel LINK_PUBLIC minifb)

add_custom_target(assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:lilray>/assets
)

add_custom_target(web_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/web
    $<TARGET_FILE_DIR:lilray>
)

get_property(targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
list(REMOVE_ITEM targets minifb assets web_assets)
foreach(target IN LISTS targets)
    target_link_libraries(${target} LINK_PUBLIC minifb)
    add_dependencies(${target} assets)
    if(EMSCRIPTEN)
        add_dependencies(${target} web_assets)
        target_link_options(${target} PRIVATE
                "-sSTRICT=1"
                "-sENVIRONMENT=web"
                "-sLLD_REPORT_UNDEFINED"
                "-sMODULARIZE=1"
                "-sALLOW_MEMORY_GROWTH=1"
                "-sALLOW_TABLE_GROWTH"
                "-sMALLOC=emmalloc"
                "-sEXPORT_ALL=1"
                "-sEXPORTED_FUNCTIONS=[\"_malloc\",\"_free\",\"_main\"]"
                "-sASYNCIFY"
                "--no-entry"
                "-sEXPORT_NAME=${target}"
        )
    endif()
endforeach()
