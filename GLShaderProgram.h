#ifndef GL_SHADER_PROGRAM_H
#define GL_SHADER_PROGRAM_H

#include <Windows.h>

#include <string>

#include <glbinding/gl33core/gl.h>
#include <glm/glm.hpp>

class GLShaderProgram
{
public:
    gl::GLuint programObject;

    GLShaderProgram(const char *vertexShaderCode, const char *fragmentShaderCode);
    GLShaderProgram(std::string vertexShaderCode, std::string fragmentShaderCode) : GLShaderProgram(vertexShaderCode.c_str(), fragmentShaderCode.c_str()) { }
    ~GLShaderProgram();

    gl::GLuint GetUniformLocation(const char* name);
    gl::GLuint GetUniformLocation(std::string name)
    {
        return GetUniformLocation(name.c_str());
    }

    void UseProgram();
    void DeleteProgram();

    void BindFragDataLocation(const char* name, gl::GLuint index);
    void BindFragDataLocation(std::string name, gl::GLuint index) { BindFragDataLocation(name.c_str(), index); }
    void SetUniformMatrix4fv(const char *name, glm::mat4 &matrix);
    void SetUniformMatrix4fv(std::string name, glm::mat4 &matrix) { SetUniformMatrix4fv(name.c_str(), matrix); }
    void SetUniformMatrix4dv(const char *name, glm::dmat4 &matrix);
    void SetUniformMatrix4dv(std::string name, glm::dmat4 &matrix) { SetUniformMatrix4dv(name.c_str(), matrix); }
    void SetUniform3dv(const char *name, glm::dvec3 &vec);
    void SetUniform3dv(std::string name, glm::dvec3 &vec) { SetUniform3dv(name.c_str(), vec); }
    void SetUniform3fv(const char *name, glm::vec3 &vec);
    void SetUniform3fv(std::string name, glm::vec3 &vec) { SetUniform3fv(name.c_str(), vec); }
    void SetUniform2fv(const char *name, glm::vec2 &vec);
    void SetUniform2fv(std::string name, glm::vec2 &vec) { SetUniform2fv(name.c_str(), vec); }
    void SetUniform1f(const char *name, gl::GLfloat v);
    void SetUniform1f(std::string name, gl::GLfloat v) { SetUniform1f(name.c_str(), v); }
    void SetUniform1d(const char *name, gl::GLdouble v);
    void SetUniform1d(std::string name, gl::GLdouble v) { SetUniform1d(name.c_str(), v); }
    void SetUniform1i(const char *name, gl::GLint v);
    void SetUniform1i(std::string name, gl::GLint v) { SetUniform1i(name.c_str(), v); }
};

class GLFilterShaderProgram : public GLShaderProgram
{
#define VERTEX_SHADER_CODE R"#(#version 400
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec2 inUV;
out vec4 position;
out vec2 uv;

void main(void)
{
    position = inPosition;
    gl_Position = position;
	uv = inUV;
})#"

public:
    static const gl::GLuint PositionLocation = 0;
    static const gl::GLuint UVLocation = 1;

    GLFilterShaderProgram(const char *fragmentShaderCode) : GLShaderProgram(VERTEX_SHADER_CODE, fragmentShaderCode) { };
    GLFilterShaderProgram(std::string fragmentShaderCode) : GLShaderProgram(VERTEX_SHADER_CODE, fragmentShaderCode) { };
};

class GLShaderProgramException : public std::runtime_error
{
public:
    std::string glMessage;

    explicit GLShaderProgramException(const std::string& message) : std::runtime_error(message) { }

    explicit GLShaderProgramException(const char *message) : std::runtime_error(message) { }

    explicit GLShaderProgramException(const std::string& message, std::string glMessage) : std::runtime_error(message)
    {
        this->glMessage = glMessage;
    }

    explicit GLShaderProgramException(const char *message, std::string glMessage) : std::runtime_error(message)
    {
        this->glMessage = glMessage;
    }
};

#endif // !GL_SHADER_PROGRAM_H
