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

# 选项：使用哪种文件对话框
option(USE_NATIVEFILEDIALOG "Use nativefiledialog for file dialogs" ON)
option(USE_GTK "Use GTK for file dialogs (may cause issues)" OFF)

# 平台特定设置
set(PLATFORM_LIBS "")

# Windows平台设置
if(WIN32)
    add_definitions(-DHAS_WINDOWS_DIALOGS)
    set(PLATFORM_LIBS comctl32 comdlg32 ole32)
endif()

# macOS平台设置
if(APPLE)
    add_definitions(-DHAS_MACOS_DIALOGS)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    set(PLATFORM_LIBS ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
endif()

# 设置基本include目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# 配置 nativefiledialog
if(USE_NATIVEFILEDIALOG)
    set(NFD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nativefiledialog)
    
    # 检查 nativefiledialog 是否存在
    if(EXISTS "${NFD_ROOT}/src/include/nfd.h")
        message(STATUS "✓ nativefiledialog found locally - enabling GUI file dialogs")
        add_definitions(-DHAS_NATIVEFILEDIALOG)
        
        # 添加 nativefiledialog 头文件目录
        include_directories(${NFD_ROOT}/src/include)
        
        # 根据平台添加对应的源文件
        set(NFD_SOURCES
            ${NFD_ROOT}/src/nfd_common.c
        )
        
        if(WIN32)
            list(APPEND NFD_SOURCES ${NFD_ROOT}/src/nfd_win.cpp)
        elseif(APPLE)
            list(APPEND NFD_SOURCES ${NFD_ROOT}/src/nfd_cocoa.m)
        elseif(UNIX)
            # Linux: 优先使用GTK版本，如果没有GTK则使用zenity
            pkg_check_modules(GTK3 gtk+-3.0)
            if(GTK3_FOUND)
                message(STATUS "  Using GTK3 backend for nativefiledialog")
                list(APPEND NFD_SOURCES ${NFD_ROOT}/src/nfd_gtk.c)
                list(APPEND PLATFORM_LIBS ${GTK3_LIBRARIES})
                include_directories(${GTK3_INCLUDE_DIRS})
                link_directories(${GTK3_LIBRARY_DIRS})
                add_definitions(${GTK3_CFLAGS_OTHER})
            else()
                message(STATUS "  Using zenity backend for nativefiledialog")
                list(APPEND NFD_SOURCES ${NFD_ROOT}/src/nfd_zenity.c)
            endif()
        endif()
        
        set(USE_NATIVEFILEDIALOG_SUCCESS ON)
    else()
        message(STATUS "⚠ nativefiledialog not found at ${NFD_ROOT}")
        set(USE_NATIVEFILEDIALOG OFF)
        set(USE_NATIVEFILEDIALOG_SUCCESS OFF)
    endif()
else()
    set(USE_NATIVEFILEDIALOG_SUCCESS OFF)
endif()

# 回退到GTK（如果用户明确要求且nativefiledialog不可用）
if(USE_GTK AND NOT USE_NATIVEFILEDIALOG_SUCCESS AND UNIX AND NOT APPLE)
    pkg_check_modules(GTK3 gtk+-3.0)
    if(GTK3_FOUND)
        message(STATUS "✓ GTK3 found - enabling native file dialogs")
        message(STATUS "  Warning: GTK may cause threading issues")
        add_definitions(-DHAS_GTK3)
        set(GTK_LIBRARIES ${GTK3_LIBRARIES})
        set(GTK_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS})
        set(GTK_LIBRARY_DIRS ${GTK3_LIBRARY_DIRS})
        list(APPEND PLATFORM_LIBS ${GTK_LIBRARIES})
    else()
        message(STATUS "⚠ GTK3 not found")
    endif()
endif()

# 最终检查
if(NOT USE_NATIVEFILEDIALOG_SUCCESS AND NOT GTK3_FOUND)
    message(STATUS "⚠ No GUI file dialog available - using console input")
    message(STATUS "  Drag and drop will still work normally")
endif()

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
    ${NFD_SOURCES}
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
    ${PLATFORM_LIBS}
)

# 添加GTK支持的include和link目录（如果需要）
if(GTK3_FOUND AND (USE_GTK OR USE_NATIVEFILEDIALOG_SUCCESS))
    target_include_directories(${PROJECT_NAME} PRIVATE ${GTK_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${GTK_LIBRARY_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
endif()

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
if(USE_NATIVEFILEDIALOG_SUCCESS)
    message(STATUS "File Dialog: nativefiledialog (GUI)")
elseif(GTK3_FOUND)
    message(STATUS "File Dialog: GTK3 (GUI)")
else()
    message(STATUS "File Dialog: Console Input")
endif()
message(STATUS "===========================")