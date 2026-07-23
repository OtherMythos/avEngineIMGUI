cmake_minimum_required(VERSION 3.5)
project(AvImguiPlugin)

file(GLOB_RECURSE srcs "src/*.cpp")

#Dear ImGui is compiled directly into each target so the plugin ships as a
#single artifact for both the dynamic and static paths.
set(imguiSrcs
    ${AV_IMGUI_DIR}/imgui.cpp
    ${AV_IMGUI_DIR}/imgui_draw.cpp
    ${AV_IMGUI_DIR}/imgui_tables.cpp
    ${AV_IMGUI_DIR}/imgui_widgets.cpp
    ${AV_IMGUI_DIR}/imgui_demo.cpp
)

#The renderer sources contain @autoreleasepool blocks for Metal, so they are
#compiled as Objective-C++ on Apple platforms (as the original project did).
if(APPLE)
    set_source_files_properties(
        src/ImguiOgre/ImguiManager.cpp
        src/ImguiOgre/ImguiRenderable.cpp
        PROPERTIES COMPILE_FLAGS "-x objective-c++"
    )
endif()

if(NOT STATIC_ONLY)
    add_library(${PROJECT_NAME} SHARED MODULE
        ${srcs}
        ${imguiSrcs}
    )
    target_include_directories(${PROJECT_NAME} PUBLIC src)

    if(APPLE)
      target_link_options(${PROJECT_NAME} PRIVATE -undefined dynamic_lookup)
    endif()

    if(MSVC)
        FIND_LIBRARY(LIB_AVCORE NAMES avCore
            PATHS ${ENGINE_SOURCE_PATH}/build/${CMAKE_BUILD_TYPE}
            PATH_SUFFIXES lib
        )

        #No SDL here: the plugin uses only the SDL scancode enum (compile time),
        #not any SDL runtime symbol, and takes all its input through the engine.
        target_link_libraries(${PROJECT_NAME}
            ${Squirrel_STD_LIBRARY}
            ${Squirrel_LIBRARY}
            ${Ogre_LIBRARY}
            ${LIB_AVCORE}
        )
    endif()
endif()

add_library(${PROJECT_NAME}_static STATIC
    ${srcs}
    ${imguiSrcs}
)
target_include_directories(${PROJECT_NAME}_static PUBLIC src)
