#include "Renderer.h"

static void GLAPIENTRY MessageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) 
{
    
    
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {

    }
    else
    {
        std::cout << "OpenGL Error: " << message << std::endl;
    }  
    
};

void Renderer::EnableDebug()
{
    //glPrimitiveRestartIndex(1000);

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}

std::vector<float> Renderer::triangles;
std::vector<float> Renderer::lines;

void Renderer::pushTriangles(std::vector<float> vertices)
{
    triangles.insert(triangles.end(), vertices.begin(), vertices.end());
}
void Renderer::pushLines(std::vector<float> vertices)
{
    lines.insert(lines.end(), vertices.begin(), vertices.end());
}


void Renderer::DrawTriangles(VertexArray& VAO, const VertexBuffer& VBO)
{
    VAO.LinkBuffer(VBO, 0);
    VAO.Bind();
    glDrawArrays(GL_TRIANGLES, 0, VAO.GetBufferCount());
}

void Renderer::DrawLines(VertexArray& VAO, const VertexBuffer& VBO)
{
    VAO.LinkBuffer(VBO, 0);
    VAO.Bind();
    glDrawArrays(GL_LINES, 0, VAO.GetBufferCount());
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
    triangles.clear();
    lines.clear();
}


void Renderer::window_size_callback(GLFWwindow* window, int width, int height)
{
    window::width = width;
    window::height = height;

}

int Renderer::window::height = 480;
int Renderer::window::width = 640;

float Renderer::simHeight = 100;
float Renderer::simWidth = Renderer::simHeight * (float(Renderer::window::width) / float(Renderer::window::height));
float Renderer::transX(float x)
{
    return x / Renderer::simWidth;
}

float Renderer::transY(float y)
{
    return y / Renderer::simHeight;
}




