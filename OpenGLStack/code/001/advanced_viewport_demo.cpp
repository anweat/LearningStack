#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

// 顶点着色器源码
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

// 片段着色器源码
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 视口参数
int viewport_x = 100;
int viewport_y = 100;
int viewport_width = 600;
int viewport_height = 400;

unsigned int shaderProgram;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Advanced Viewport Demo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义顶点数据 - 包含位置和颜色
    float vertices[] = {
        // 位置              // 颜色
        -1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 红色 - 左上 (NDC边界)
         1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // 绿色 - 右上 (NDC边界)
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // 蓝色 - 右下 (NDC边界)
        -1.0f, -1.0f, 0.0f,  1.0f, 1.0f, 0.0f   // 黄色 - 左下 (NDC边界)
    };

    unsigned int indices[] = {
        0, 1, 2,  // 第一个三角形
        2, 3, 0   // 第二个三角形
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    std::cout << "=== 高级视口演示程序 ===" << std::endl;
    std::cout << "按键说明:" << std::endl;
    std::cout << "W/S - 增加/减少 viewport_y" << std::endl;
    std::cout << "A/D - 减少/增加 viewport_x" << std::endl;
    std::cout << "Q/E - 减少/增加 viewport_width" << std::endl;
    std::cout << "Z/C - 减少/增加 viewport_height" << std::endl;
    std::cout << "R - 重置视口为全屏" << std::endl;
    std::cout << "T - 设置视口到负坐标区域" << std::endl;
    std::cout << "I - 显示当前视口信息" << std::endl;
    std::cout << "ESC - 退出程序" << std::endl;
    std::cout << "\n注意观察当视口参数为负值时的渲染效果!" << std::endl;

    // 设置初始视口
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        // 绘制红色边框来标识视口边界
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(3.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // 填充内部
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 注意：这里不自动更改我们的自定义视口
    // 这样可以看到视口参数的实际影响
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        bool viewport_changed = true;
        
        switch (key) {
            case GLFW_KEY_W:
                viewport_y += 10;
                break;
            case GLFW_KEY_S:
                viewport_y -= 10;
                break;
            case GLFW_KEY_A:
                viewport_x -= 10;
                break;
            case GLFW_KEY_D:
                viewport_x += 10;
                break;
            case GLFW_KEY_Q:
                viewport_width += 10;
                if (viewport_width > (int)SCR_WIDTH) viewport_width = SCR_WIDTH;
                break;
            case GLFW_KEY_E:
                viewport_width -= 10;
                if (viewport_width < 50) viewport_width = 50;
                break;
            case GLFW_KEY_Z:
                viewport_height += 10;
                if (viewport_height > (int)SCR_HEIGHT) viewport_height = SCR_HEIGHT;
                break;
            case GLFW_KEY_C:
                viewport_height -= 10;
                if (viewport_height < 50) viewport_height = 50;
                break;
            case GLFW_KEY_R:
                viewport_x = 0;
                viewport_y = 0;
                viewport_width = SCR_WIDTH;
                viewport_height = SCR_HEIGHT;
                break;
            case GLFW_KEY_T:
                // 设置视口到负坐标区域
                viewport_x = -200;
                viewport_y = -150;
                viewport_width = SCR_WIDTH;
                viewport_height = SCR_HEIGHT;
                break;
            case GLFW_KEY_I:
                std::cout << "\n当前视口参数:" << std::endl;
                std::cout << "glViewport(" << viewport_x << ", " << viewport_y 
                          << ", " << viewport_width << ", " << viewport_height << ")" << std::endl;
                
                std::cout << "\n坐标变换示例 (NDC -> 屏幕):" << std::endl;
                std::cout << "NDC(-1, 1) -> 屏幕(" 
                          << (int)((-1 + 1) * (viewport_width/2.0) + viewport_x) << ", "
                          << (int)(( 1 + 1) * (viewport_height/2.0) + viewport_y) << ")" << std::endl;
                          
                std::cout << "NDC(1, -1) -> 屏幕(" 
                          << (int)(( 1 + 1) * (viewport_width/2.0) + viewport_x) << ", "
                          << (int)((-1 + 1) * (viewport_height/2.0) + viewport_y) << ")" << std::endl;
                break;
            default:
                viewport_changed = false;
        }
        
        if (viewport_changed) {
            glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
            
            // 显示当前视口参数
            std::cout << "视口已更新为: glViewport(" << viewport_x << ", " << viewport_y 
                      << ", " << viewport_width << ", " << viewport_height << ")" << std::endl;
        }
    }
}