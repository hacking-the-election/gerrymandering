#include "../include/graphics.h"

#ifdef NVIDIA_DEDICATED
extern "C" 
{
    __attribute__((visibility("default"))) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif


namespace hte {
namespace gl {


const std::string Canvas::vertexShader = R"(#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 fillColor;
layout(location = 2) in vec4 outlineColor;

uniform mat4 MVP;

out vec4 iFillColor;
out vec4 iOutlineColor;

#define PI 3.141592653589793238462f

void main()
{
    iFillColor = fillColor;
    iOutlineColor = outlineColor;
    gl_Position = MVP * position;
})";

const std::string Canvas::fragmentShader = R"(#version 330 core
out vec4 color;

in vec4 iFillColor;
in vec4 iOutlineColor;

uniform int colorType;

void main()
{
    if (colorType == 0) {
        color = iFillColor;
    }
    else
    {
        color = iOutlineColor;
    }

    // color = iOutlineColor;
})";


GLBufferBase::GLBufferBase(GLuint type, const void* data, GLuint size) : bufferType(type)
{
    glGenBuffers(1, &bufferId);
    glBindBuffer(bufferType, bufferId);
    glBufferData(bufferType, size, data, GL_STATIC_DRAW);
}

GLBufferBase::~GLBufferBase()
{
    glDeleteBuffers(1, &bufferId);
}

void GLBufferBase::bind() const
{
    glBindBuffer(bufferType, bufferId);
}

void GLBufferBase::unbind() const
{
    glBindBuffer(bufferType, 0);
}


//==========

GLVertexArrayObject::GLVertexArrayObject()
{
    glGenVertexArrays(1, &vertexArrayId);
}

GLVertexArrayObject::~GLVertexArrayObject()
{
    glDeleteVertexArrays(1, &vertexArrayId);
}

void GLVertexArrayObject::addBuffer(const GLVertexBuffer& vb)
{
    bind();
    vb.bind();
    GLuint offset = 0;

    for (size_t i = 0; i < vb.attributes.size(); i++)
    {
        const auto& attr = vb.attributes[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attr.count, attr.type, attr.normalized, vb.stride, (const void*)offset);
        offset += attr.count * attr.getTypeSize();
    }
}

void GLVertexArrayObject::bind() const
{
    glBindVertexArray(vertexArrayId);
}

void GLVertexArrayObject::unbind() const
{
    glBindVertexArray(0);
}


void Canvas::init(const std::string& title)
{
    if (!glfwInit())
    {
        errorCallback(-1, "Failed to init glfw");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    glfwSetErrorCallback(Canvas::errorCallback);

    if (!window)
    {
        errorCallback(-1, "Could not instantiate window");
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    shaderProgram = CreateProgram(vertexShader, fragmentShader);
}

}
}
