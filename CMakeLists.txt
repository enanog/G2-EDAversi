cmake_minimum_required(VERSION 3.17.0)
project(main VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# From "Working with CMake" documentation:
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # AddressSanitizer (ASan)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # UndefinedBehaviorSanitizer (UBSan)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

add_executable(main
    main.cpp
    model.cpp
    controller.cpp

    view/view.cpp
    view/ui_components.cpp
    view/board_renderer.cpp
    view/game_overlay.cpp
    view/menu_system.cpp
    view/settings_overlay.cpp

    ai/ai_factory.cpp
    ai/ai_easy.cpp
    ai/ai_normal.cpp
    ai/ai_hard.cpp
    ai/ai_extreme.cpp
    ai/opening_book.cpp
    ai/transposition_table.cpp
)
# Raylib
find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_include_directories(main PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${raylib_LIBRARIES} glfw)

# Threads (portable): ensures -pthread on GCC/MinGW and links against proper threading lib
find_package(Threads REQUIRED)
target_link_libraries(main PRIVATE Threads::Threads)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # From "Working with CMake" documentation:
    target_link_libraries(main PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(main PRIVATE m ${CMAKE_DL_LIBS} pthread GL rt X11)
endif()

# MSVC-specific compile options: enable proper exception handling for threads
if (MSVC)
    # /EHsc enables C++ exceptions model compatible with threads
    target_compile_options(main PRIVATE /EHsc)
endif()

# MinGW / GCC: ensure -pthread is used if not already (Threads::Threads usually covers this,
# but this guard can help for some toolchains)
if (MINGW)
    target_compile_options(main PRIVATE -pthread)
    target_link_libraries(main PRIVATE -pthread)
endif()
