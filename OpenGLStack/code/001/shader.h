#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <glad/glad.c>
#include <iostream>

static GLuint loadShader(const std::string &vertexPath,
                         const std::string &fragmentPath)
{
    auto read = [](const std::string &fp) -> std::string
    {
        std::ifstream f(fp);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    };
    std::string vStr = read(vertexPath);
    std::string fStr = read(fragmentPath);
    const char *vSrc = vStr.c_str();
    const char *fSrc = fStr.c_str();

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vSrc, nullptr);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fSrc, nullptr);
    glCompileShader(f);

    GLuint id = glCreateProgram();
    glAttachShader(id, v);
    glAttachShader(id, f);
    glLinkProgram(id);
    glDeleteShader(v);
    glDeleteShader(f);
    return id;
}
