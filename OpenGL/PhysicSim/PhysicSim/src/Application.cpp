#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Renderer.h"

namespace Positions {

    float trianglePositions[] = {
        -0.1f, -0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,
            0.1f, -0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,
            0.1f,  0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,

            0.5f, -0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
            0.6f, -0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
            0.6f,  0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
    };

    float linePositions[] = {
         -0.5,  0.5, 0.0f, 0.1f, 0.1f, 0.9f, 1.0f,
          0.5,  0.5, 0.0f, 0.1f, 0.1f, 0.9f, 1.0f,
    };
}

int main(void)
{
    
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;

    Renderer renderer;


    
    VertexArray VAO;
    VertexBuffer VBO1(Positions::trianglePositions, sizeof(Positions::trianglePositions));
    VertexBuffer VBO2(Positions::linePositions, sizeof(Positions::linePositions));

    Shader shader("res/shaders/Basic.shader");

    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        renderer.Clear();

        renderer.DrawTriangles(VAO, VBO1);
      
        renderer.DrawLines(VAO, VBO2);

        /* Swap Buffers and poll IO events */
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }


    glfwTerminate();
    return 0;
}