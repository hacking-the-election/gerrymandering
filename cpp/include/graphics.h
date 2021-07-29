#ifndef _HTE_GRAPHICS_H
#define _HTE_GRAPHICS_H

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include "geometry.h"

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>


namespace hte {
namespace gl {


struct Style
{
    int outlineThickness;
    
    // use OpenGL structures for colors or shaders
    bool isFilled;
    bool isOutlined;

    // RGBcolor outline;
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
            case GL_DOUBLE: return sizeof(GLdouble);
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
    GLuint bufferId;

private:
    GLuint bufferType;
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


template<typename T>
void CheckAndSetAABB(std::unordered_map<Bounds, T>& AABB, const Point2d<T>& check)
{
    // check X coordinate against bounding box, adjust box if necessary
    if (check.x < AABB[Bounds::Left]) AABB[Bounds::Left] = check.x;
    else if (check.x > AABB[Bounds::Right]) AABB[Bounds::Right] = check.x;

    // check Y coordinate against bounding box, adjust box if necessary
    if (check.y < AABB[Bounds::Bottom]) AABB[Bounds::Bottom] = check.y;
    else if (check.y > AABB[Bounds::Top]) AABB[Bounds::Top] = check.y;
}


class GeoBuffer
{
public:
    GeoBuffer() {}

    template<typename T>
    void pushBuffer(const Polygon<T>& p, const Style& style = Style())
    {
        buffer.push_back({p, style});
    };


    template<typename T>
    void pushBuffer(const MultiPolygon<T>& mp, const Style& style = Style())
    {
        for (const Polygon<T>& p : mp) pushBuffer(p, style);
    };

    // void ();

    // this is the ugliest function signature known to man. please kill it
    std::tuple<std::shared_ptr<GLVertexBuffer>, std::shared_ptr<GLIndexBuffer>, std::shared_ptr<GLIndexBuffer>, int, int> buildGLBuffers()
    {
        std::vector<GLuint> tri_indices, line_indices;
        std::vector<GLdouble> vertices;

        std::unordered_map<Bounds, double> AABB;

        // track current number of elements in vb, doubles as the indexbuffer offset counter
        GLuint totalBuffSize = 0;

        for (auto& [poly, style] : buffer)
        {
            GLuint buffSize = 0;

            for (size_t i = 0; i < poly.size(); i++)
            {
                poly[i].forceValid();
                buffSize += poly[i].size();
                for (const Point2d<double>& p : poly[i])
                {
                    // holes shouldn't affect the bounding box
                    if (i == 0) CheckAndSetAABB(AABB, p);
                    vertices.push_back(p.x);
                    vertices.push_back(p.y);
                }
            }

            // auto& index = (style.)
            if (style.isFilled)
            {
                // triangulate polygon, place them into the filled index buffer
                const std::vector<GLuint>& triangles = mapbox::earcut<GLuint>(poly);      

                // offset each index in `triangles` by the number of vertices that have been processed 
                std::transform(triangles.begin(), triangles.end(), std::back_inserter(tri_indices), [totalBuffSize](GLuint ind) {
                    return ind + totalBuffSize;
                });
            }
            
            if (style.isOutlined)
            {
                line_indices.resize(line_indices.size() + buffSize);
                std::iota(std::prev(line_indices.end()) - buffSize, line_indices.end(), totalBuffSize);
            }

            totalBuffSize += buffSize;
        }

        // sussy baka!
        std::shared_ptr<GLVertexBuffer> vb = std::make_shared<GLVertexBuffer>(vertices.data(), sizeof(double) * vertices.size());
        vb->addAttribute(2, GL_DOUBLE);

        std::shared_ptr<GLIndexBuffer> tri_ib = std::make_shared<GLIndexBuffer>(tri_indices.data(), tri_indices.size() * sizeof(GLuint));
        std::shared_ptr<GLIndexBuffer> line_ib = std::make_shared<GLIndexBuffer>(line_indices.data(), line_indices.size() * sizeof(GLuint));
        return {vb, tri_ib, line_ib, tri_indices.size(), line_indices.size()};
    };

    void clear() {buffer.clear();}

private:
    std::vector<std::pair<Polygon<double>, Style>> buffer;
};


class Canvas
{
public:
    template<typename ...Args>
    void pushGeometry(Args&&... args) {geoBuffer.pushBuffer(std::forward<Args>(args)...);}

    static Canvas* GetInstance()
    {
        static Canvas r;
        return &r;
    }

    void init(const std::string& title = "htevis");
    void deinit();

    inline GLFWwindow* getWindow() {return window;}

    void renderCurrent()
    {
        const auto [vb, tri_ib, line_ib, tri_count, line_count] = geoBuffer.buildGLBuffers();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // if (width > height)
        glm::mat4 ortho = glm::ortho(0.f, 10.f * width / height, 0.f, 10.f, -1.f, 1.f);

        GLuint fragmentColorLocation = glGetUniformLocation(shaderProgram, "inColor");
        glUniform4f(fragmentColorLocation, 1.f, 0.5f, 0.5f, 0.5f);
    
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &ortho[0][0]);

        glLineWidth(8.f);
        glEnable(GL_LINE_SMOOTH);
        
        GLVertexArrayObject va;
        va.bind();
        va.addBuffer(*vb);

        tri_ib->bind();
        glDrawElements(GL_TRIANGLES, tri_count, GL_UNSIGNED_INT, 0);

        line_ib->bind();
        glDrawElements(GL_LINE_STRIP, line_count, GL_UNSIGNED_INT, 0);
        
        
        glfwSwapBuffers(getWindow());
        glfwPollEvents();

        va.unbind();
        vb->unbind();
        return;
    };


    static void errorCallback(int code, const char* glfwMsg) {}


    static GLuint CompileShader(const std::string& shaderSource, GLuint type)
    {
        unsigned int id = glCreateShader(type);
        const char* src = shaderSource.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        // error handling
        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);

        if (!result)
        {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

            // read error into message
            char* message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);

            std::cout << "Failed to compile shader." << std::endl;
            std::cout << message << std::endl;

            glDeleteShader(id);
            return -1;
        }

        return id;
    }


    static GLuint CreateProgram(const std::string& vertexShader, const std::string& fragmentShader)
    {
        GLuint program = glCreateProgram();
        GLuint vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
        GLuint fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    const static std::string vertexShader;
    const static std::string fragmentShader;

private:
    GLFWwindow* window;
    GLVertexArrayObject va = GLVertexArrayObject();
    GLuint shaderProgram;

    GeoBuffer geoBuffer;
    const static int width = 640, height = 480;


    Canvas()
    {
        this->init();
    };

    ~Canvas()
    {
        glDeleteProgram(shaderProgram);
    }
};

}
}

#endif