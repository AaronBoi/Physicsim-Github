#include "Core.h"
#include <iostream>

void Core::window_size_callback(GLFWwindow* window, int width, int height)
{
    window::width = width;
    window::height = height;
}

const float Core::dt = float(1.0 / 60.0);





int Core::window::height = 480;
int Core::window::width = 640;

float Core::simHeight = 100;
float Core::simWidth = Core::simHeight * (float(Core::window::width) / float(Core::window::height));
float Core::transX(float x)
{
    return x / Core::simWidth;
}

float Core::transY(float y)
{
    return y / Core::simHeight;
}

float Core::circleData[];

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

void Core::EnableDebug()
{
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}