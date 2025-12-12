cmake_minimum_required(VERSION 3.10)
project(OpenGLStack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 库目录
link_directories(${CMAKE_SOURCE_DIR}/lib)

# 编译 GLAD 为静态库（只编译一次）
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 自动检测源文件目录中的所有 .cpp 文件
file(GLOB CPP_FILES "${CMAKE_SOURCE_DIR}/*.cpp")

# 为每个 .cpp 文件创建一个可执行文件
foreach(CPP_FILE ${CPP_FILES})
    get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
    
    # 创建可执行文件（不再包含 glad.c）
    add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
    
    # 链接 GLAD 库和其他库
    target_link_libraries(${EXECUTABLE_NAME}
        glad
        ${CMAKE_SOURCE_DIR}/lib/glfw3dll.lib
        opengl32
        gdi32
        user32
    )
    
    # 复制 DLL 到输出目录
    file(GLOB DLL_FILES "${CMAKE_SOURCE_DIR}/lib/*.dll")
    foreach(DLL_FILE ${DLL_FILES})
        add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_FILE} $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
            COMMENT "Copying ${DLL_FILE}")
    endforeach()
    
    # 复制所有文件和文件夹（排除 .vscode, build, include, util, .gitignore, *.md）
    file(GLOB_RECURSE ALL_FILES RELATIVE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/*")
    
    foreach(FILE ${ALL_FILES})
        # 排除特定文件夹和文件
        if(NOT FILE MATCHES "^\\.vscode/" AND
           NOT FILE MATCHES "^build/" AND
           NOT FILE MATCHES "^include/" AND
           NOT FILE MATCHES "^util/" AND
           NOT FILE MATCHES "^lib/" AND
           NOT FILE MATCHES "^\\.gitignore$" AND
           NOT FILE MATCHES "\\.md$")
            
            set(SRC_FILE "${CMAKE_SOURCE_DIR}/${FILE}")
            set(DEST_FILE "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${FILE}")
            
            # 如果是目录，复制目录；如果是文件，复制文件
            if(IS_DIRECTORY "${SRC_FILE}")
                add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRC_FILE}" "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${FILE}"
                    COMMENT "Copying directory ${FILE}")
            else()
                add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE}" "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${FILE}"
                    COMMENT "Copying ${FILE}")
            endif()
        endif()
    endforeach()
endforeach()






