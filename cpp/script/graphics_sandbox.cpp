#include <iostream>
#include <unistd.h>


#include "../include/hte_common.h"


using namespace std;
using namespace hte;
using namespace hte::gl;



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


const static std::string vertexShader = R"(#version 330 core
    layout(location = 0) in vec4 position;
    void main()
    {
        gl_Position = position;
    }
)";

const static std::string fragmentShader = R"(
    #version 330 core
    out vec4 color;
    uniform vec4 inColor;

    void main()
    {
        color = inColor;
    }
)";


int main(int argc, char* argv[]) {
    // read files
    // DataParser dataParser({
    //     {FileType::BLOCK_GEO, argv[1]},
    //     {FileType::BLOCK_DEMOGRAPHICS, argv[2]},
    //     {FileType::PRE_GEO, argv[3]},
    //     {FileType::PRE_DEMOGRAPHICS, argv[4]},
    // });

    // State state = State();
    // dataParser.parseToState(state);

    std::vector<Polygon<double>> polygons;

    Canvas* ctx = Canvas::GetInstance();
    ctx->init();

    for (const auto& p : polygons)
    {
        ctx->pushGeometry(p, Style{1});
    }

    // c->renderCurrent();


    // glfwSetErrorCallback(ContextManager::errorCallback);

    // Create shader program from the individual shaders
    GLuint program = CreateProgram(vertexShader, fragmentShader);
    glUseProgram(program);

    // may be == -1 if the uniform name is not found
    int fragmentColorLocation = glGetUniformLocation(program, "inColor");

    // Specify unique positions of vertices
    float positions[] = {
        -0.5f, -0.5f,
        0.5, -0.5,
        0.5, 0.5,
        -0.5, 0.5
    };

    // Ordered list of indices of vertex positions that represent the shapes to be drawn
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // GLuint vao;
    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);
    GLVertexArrayObject va;
    va.bind();

    GLVertexBuffer vb(positions, 4 * 2 * sizeof(float));
    vb.addAttribute(2, GL_FLOAT);
    va.addBuffer(vb);

    GLIndexBuffer ib(indices, 6 * sizeof(GLuint));

    // animated uniform
    float r = 0.f;
    float increment = 0.05f;

    while (!glfwWindowShouldClose(ctx->getWindow()))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (r >= 1.0f)
            increment = -0.05f;
        else if (r <= 0.0f)
            increment = 0.05f;

        r += increment;

        glUseProgram(program);
        glUniform4f(fragmentColorLocation, r, 0.0f, 0.f, 1.f);

        va.bind();
        // vb.bind();
        ib.bind();

        // glLineWidth(4.f);
        // glEnable(GL_LINE_SMOOTH);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(ctx->getWindow());
        
        glfwPollEvents();
    }
    

    glDeleteProgram(program);
    // usleep(5 * 1000000);

    return 0;
}
