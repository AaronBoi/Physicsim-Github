#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <iostream>

#include "shader.h"
#include "VertexBuffer.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "Physics.h"




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

    Renderer::EnableDebug();

    std::vector<float> testTriangles = {
        -0.1f, -0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,
        0.1f, -0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,
        0.1f,  0.1f, 0.0f, 0.1f, 0.5f, 0.1f, 1.0f,

        0.5f, -0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
        0.6f, -0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
        0.6f,  0.2f, 0.0f, 0.9f, 0.3f, 0.4f, 1.0f,
    };

    std::vector<float> testLines = {
        -0.5,  0.5, 0.0f, 0.1f, 0.1f, 0.9f, 1.0f,
        0.5,  0.5, 0.0f, 0.1f, 0.1f, 0.9f, 1.0f,
    };
    

    Renderer::pushTriangles(testTriangles);
    Renderer::pushLines(testLines);
    
 
    Physics::SetupScene();

    VertexArray VAO;
    


    Shader shader("res/shaders/Basic.shader");

    
    

    // Main while loop
    while (!glfwWindowShouldClose(window))
    {

        Renderer::Clear();
        //Physics::particles[0].rgbt.G += 0.001;
        for (int i = 0; i < Physics::particles.size(); i++)
        {

            Physics::particles[i].Simulate(Physics::dt);
            Physics::particles[i].Draw();

        }

        VertexBuffer VBO1(Renderer::triangles.data(), Renderer::triangles.size() * sizeof(float));
        VertexBuffer VBO2(Renderer::lines.data(), Renderer::lines.size() * sizeof(float));

        Renderer::DrawTriangles(VAO, VBO1);
        Renderer::DrawLines(VAO, VBO2);

        /* Swap Buffers and poll IO events */
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }


    glfwTerminate();
    return 0;
}