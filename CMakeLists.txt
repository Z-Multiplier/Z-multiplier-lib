cmake_minimum_required(VERSION 3.15)
project(Zmultiplier VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX /EHsc)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_library(Zmultiplier STATIC
    src/core/Color.cpp
    src/core/Canvas.cpp
    src/window/Window.cpp
    src/core/Logger.cpp
    src/utils/Utils.cpp
    src/painter/Painter.cpp
    src/core/EntryPoint.cpp
    src/assets/Image.cpp
    src/assets/Font.cpp
)

target_link_libraries(Zmultiplier PRIVATE
    msimg32
    gdi32
    user32
    gdiplus
)

target_include_directories(Zmultiplier PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_compile_definitions(Zmultiplier PRIVATE NOMINMAX)
    target_link_libraries(Zmultiplier PRIVATE gdi32 user32)
endif()

install(TARGETS Zmultiplier
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/Zmultiplier DESTINATION include)

add_subdirectory(examples)