#include <iostream>
#include <cmath>

// 包含GLAD和GLFW头文件
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

// ---------- 1. 窗口回调 ----------
void framebuffer_size_callback(GLFWwindow *w, int w_, int h_)
{
    glViewport(0, 0, w_, h_);
}

// ---------- 2. 顶点数据（一个彩色三角形） ----------
float vertices[] = {
    //  x,     y,     z,     r,     g,     b
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f};

// ---------- 3. main ----------
int main()
{
    // 3.1 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "GL 60Hz", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3.2 加载 GL 函数

    if (!gladLoadGL())
    { // 返回 0 表示失败
        std::cerr << "gladLoadGL error\n";
        return -1;
    }
    glViewport(0, 0, 800, 600);

    // 3.3 创建 VAO/VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 0 = pos, 3 * float
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 1 = color, 3 * float
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // 3.4 加载着色器（你已有的 loadShader）
    GLuint prog = loadShader("shader.vert", "shader.frag");

    // 3.5 开垂直同步 → 驱动自动锁 60 FPS
    glfwSwapInterval(1);

    // 3.6 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window); // 阻塞到 16.666 ms
        glfwPollEvents();
    }

    // 3.7 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}