#include "../include/graphics.h"


namespace hte {
namespace gl {


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

    window = glfwCreateWindow(600, 600, title.c_str(), NULL, NULL);

    if (!window)
    {
        errorCallback(-1, "Could not instantiate window");
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

}
}
