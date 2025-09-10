cmake_minimum_required(VERSION 3.16)
project(MoneyTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ------------------------------
# GLFW
# ------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# ------------------------------
# GLAD
# ------------------------------
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
)
FetchContent_MakeAvailable(glad)

add_library(glad STATIC ${glad_SOURCE_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${glad_SOURCE_DIR}/include)

# ------------------------------
# ImGui
# ------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# ------------------------------
# Windows resource file
# ------------------------------
if(WIN32)
    set(RESOURCE_FILES resources/app.rc)
    enable_language(RC)
endif()

# ------------------------------
# Create executable
# ------------------------------
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/encrypter.cpp
    src/savingFunctions.cpp
    ${IMGUI_SOURCES}
    ${RESOURCE_FILES}
)

# ------------------------------
# Include directories
# ------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glad_SOURCE_DIR}/include
    external/stb   # keep this local for now unless you want me to FetchContent stb too
    src
)

# ------------------------------
# Link libraries
# ------------------------------
target_link_libraries(${PROJECT_NAME}
    glfw
    glad
)

# ------------------------------
# Windows-specific libraries
# ------------------------------
if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 bcrypt)

    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
    elseif(MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()

# ------------------------------
# For MinGW static linking
# ------------------------------
if(MINGW)
    target_link_libraries(${PROJECT_NAME}
        -static-libgcc
        -static-libstdc++
        -static
        -lpthread
    )
endif()

# ------------------------------
# Copy application icon
# ------------------------------
file(COPY ${CMAKE_SOURCE_DIR}/resources/app_icon.png
     DESTINATION ${CMAKE_BINARY_DIR}/resources)
