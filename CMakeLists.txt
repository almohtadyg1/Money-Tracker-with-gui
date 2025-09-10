cmake_minimum_required(VERSION 3.16)
project(MoneyTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable GLFW docs, tests and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Add GLFW
add_subdirectory(external/glfw)

# Add GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PRIVATE external/glad/include)

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

# Windows resource file
if(WIN32)
    set(RESOURCE_FILES resources/app.rc)
    enable_language(RC)
endif()

# Create executable - WIN32 keyword for GUI apps on Windows
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/encrypter.cpp
    src/savingFunctions.cpp
    ${IMGUI_SOURCES}
    ${RESOURCE_FILES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    external/glad/include
    external/stb
    src
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    glfw
    glad
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 bcrypt)

    if(MSVC)
        # Only MSVC understands these
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
    elseif(MINGW)
        # MinGW equivalent
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()

# For MinGW static linking
if(MINGW)
    target_link_libraries(${PROJECT_NAME}
        -static-libgcc
        -static-libstdc++
        -static
        -lpthread
    )
endif()

# Copy application icon to build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources/app_icon.png
     DESTINATION ${CMAKE_BINARY_DIR}/resources)