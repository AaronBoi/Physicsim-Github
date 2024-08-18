#include "Renderer.h"



Renderer::Renderer()
{
    EnableDebug();
}

void GLAPIENTRY MessageCallback(
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
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
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
}




