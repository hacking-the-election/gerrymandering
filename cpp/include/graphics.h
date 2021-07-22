#ifndef _HTE_GRAPHICS_H
#define _HTE_GRAPHICS_H

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include "geometry.h"

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cassert>


namespace hte {
namespace gl {


struct Style
{
    int outlineThickness;
    
    // use OpenGL structures for colors or shaders
    // RGBcolor fill;
    // RGBcolor outline;
};


class GeoBuffer
{
public:
    GeoBuffer() {}

    template<typename T>
    void pushBuffer(const Polygon<T>& p, const Style& style = Style());
    
    template<typename T>
    void pushBuffer(const MultiPolygon<T>& p, const Style& style = Style());
    // void ();

    void clear() {buffer.clear();}

private:
    std::vector<std::pair<LinearRing<double>, Style>> buffer;
};


// stores data about a single vertex attrib
struct GLVertexBufferAttrib
{
    GLuint type;
    GLuint count;
    unsigned char normalized;

    GLuint getTypeSize() const
    {
        switch (type)
        {
            case GL_FLOAT: return sizeof(GLfloat);
        }

        assert(false);
        return 0;
    }
};


class GLBufferBase
{
public:
    GLBufferBase(GLuint type, const void* data, GLuint size);
    ~GLBufferBase();

    void bind() const;
    void unbind() const;

private:
    GLuint bufferType;
    GLuint bufferId;
    GLuint count;
};


class GLIndexBuffer : public GLBufferBase
{
public:
    GLIndexBuffer(const void* data, GLuint size) : GLBufferBase(GL_ELEMENT_ARRAY_BUFFER, data, size) {}
};


class GLVertexBuffer : public GLBufferBase
{
public:
    GLVertexBuffer(const void* data, GLuint size) : GLBufferBase(GL_ARRAY_BUFFER, data, size) {};
    // ~GLVertexBuffer();

    void addAttribute(GLuint count, GLuint type)
    {
        attributes.push_back({type, count, GL_FALSE});
        stride += attributes.rbegin()->getTypeSize() * count;
        std::cout << stride << std::endl;
   }

private:
    std::vector<GLVertexBufferAttrib> attributes;
    GLuint stride = 0;

friend class GLVertexArrayObject;
};


class GLVertexArrayObject
{
public:
    GLVertexArrayObject();
    ~GLVertexArrayObject();

    void addBuffer(const GLVertexBuffer& vb);

    void bind() const;
    void unbind() const;

private:
    GLuint vertexArrayId;
};


class Canvas
{
public:
    template<typename ...Args>
    void pushGeometry(Args&&... args) {geoBuffer.pushBuffer(std::forward<Args>(args)...);}

    // void pushGeometry();

    static Canvas* GetInstance()
    {
        static Canvas r;
        return &r;
    }

    void init(const std::string& title = "htevis");
    void deinit();

    inline GLFWwindow* getWindow() {return window;}
    // void drawCurrent()
    // {
        
    // };

    static void errorCallback(int code, const char* glfwMsg) {}

private:
    GLFWwindow* window;
    GeoBuffer geoBuffer;

    // this really shouldn't be part of the context-managing class?
    void draw() {
        glClear(GL_COLOR_BUFFER_BIT);

        // bind correct buffers
        // va.bind();
        // ib.bind();
        
        // setup options

        // glDrawElements(GL_TRIANGLES, vb.getCount(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
    }

    Canvas() {};
};

}
}

#endif