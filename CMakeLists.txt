cmake_minimum_required(VERSION 3.16)
project(FFTImageProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找必要的包
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# 查找GLFW和GLEW
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

# 设置include目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ImGui
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.h")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends)
    
    set(IMGUI_SOURCES
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_opengl3.cpp
    )
    message(STATUS "✓ ImGui found")
else()
    message(FATAL_ERROR "ImGui not found!")
endif()

# ImPlot
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/implot.h")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot)
    
    set(IMPLOT_SOURCES
        third_party/implot/implot.cpp
        third_party/implot/implot_items.cpp
    )
    message(STATUS "✓ ImPlot found")
else()
    message(FATAL_ERROR "ImPlot not found!")
endif()

# 源文件
set(SOURCES
    main.cpp
    src/ImageProcessor.cpp
    src/OpenGLRenderer.cpp
    src/GUI.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLEW
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    ${GLFW3_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# 设置路径
target_link_directories(${PROJECT_NAME} PRIVATE
    ${GLFW3_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra
        -Wno-unused-parameter
        -Wno-unused-variable
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "===========================")
