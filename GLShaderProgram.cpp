#include "GLShaderProgram.h"

using namespace gl;

GLuint CompileShader(const char* code, GLenum type)
{
    const char* codes[1] = { code };

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, codes, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char message[4096];
        SecureZeroMemory(&message[0], sizeof(message));
        glGetShaderInfoLog(shader, sizeof(message), NULL, message);
        throw GLShaderProgramException("Cannot compile shader", std::string(message));
    }

    return shader;
}

GLShaderProgram::GLShaderProgram(const char * vertexShaderCode, const char * fragmentShaderCode)
{
    GLuint vertexShader = CompileShader(vertexShaderCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(fragmentShaderCode, GL_FRAGMENT_SHADER);

    this->programObject = glCreateProgram();
    glAttachShader(this->programObject, vertexShader);
    glAttachShader(this->programObject, fragmentShader);
    glLinkProgram(this->programObject);

    GLint status;
    glGetProgramiv(this->programObject, GL_LINK_STATUS, &status);
    if (!status)
    {
        char message[4096];
        SecureZeroMemory(&message[0], sizeof(message));
        glGetProgramInfoLog(this->programObject, sizeof(message), NULL, message);
        throw GLShaderProgramException("Cannot link program", std::string(message));
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLShaderProgram::~GLShaderProgram()
{
    if (this->programObject)
    {
        DeleteProgram();
    }
}

GLuint GLShaderProgram::GetUniformLocation(const char *name)
{
    return glGetUniformLocation(this->programObject, name);
}

void GLShaderProgram::UseProgram()
{
    glUseProgram(this->programObject);
}

void GLShaderProgram::DeleteProgram()
{
    if (this->programObject)
    {
        glDeleteProgram(this->programObject);
        this->programObject = 0;
    }
}

void GLShaderProgram::BindFragDataLocation(const char * name, GLuint index)
{
    glBindFragDataLocation(this->programObject, index, name);
}

void GLShaderProgram::SetUniformMatrix4dv(const char *name, glm::dmat4 & matrix)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniformMatrix4dv(location, 1, GL_FALSE, (GLdouble *)&matrix[0][0]);
}

void GLShaderProgram::SetUniformMatrix4fv(const char *name, glm::mat4 & matrix)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat *)&matrix[0][0]);
}

void GLShaderProgram::SetUniform3dv(const char * name, glm::dvec3 & vec)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform3dv(location, 1, (GLdouble *)&vec[0]);
}

void GLShaderProgram::SetUniform3fv(const char * name, glm::vec3 & vec)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform3fv(location, 1, (GLfloat *)&vec[0]);
}

void GLShaderProgram::SetUniform2fv(const char * name, glm::vec2 & vec)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform2fv(location, 1, (GLfloat *)&vec[0]);
}

void GLShaderProgram::SetUniform1f(const char * name, gl::GLfloat v)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform1f(location, v);
}

void GLShaderProgram::SetUniform1d(const char * name, gl::GLdouble v)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform1d(location, v);
}

void GLShaderProgram::SetUniform1i(const char * name, gl::GLint v)
{
    GLuint location = glGetUniformLocation(this->programObject, name);
    glUniform1i(location, v);
}
