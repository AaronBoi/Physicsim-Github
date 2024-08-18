#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Physics.h"
#include "Vector2.h"
#include "Particle.h"
#include "Core.h"

int main(void)
{
    
    

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(Core::window::width, Core::window::height, "PhysicsSim", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwSetWindowSizeCallback(window, Core::window_size_callback);
    

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    

    Physics::particles.reserve(50);
    //particles.resize(1);

    Physics::SetupScene(Physics::particles);
   
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Core::circleData), Core::circleData, GL_STATIC_DRAW);
    //glDrawArrays();
    //std::cout << Core::simWidth << std::endl;
    
    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        for (int i = 0; i < Physics::particles.size(); i++)
        {

            Physics::particles[i].Simulate(Core::dt);
            Physics::particles[i].Draw();

        }
        /*
        
        glColor3f(0.0, 1.0, 1.0);
        glBegin(GL_TRIANGLES);
        
        glVertex2f(-0.5, -0.5);
        glVertex2f(0.5, -0.5);
        glVertex2f(0.5, 0.5);
        
        glEnd();
        */
        /* Swap Buffers and poll IO events */
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }


    glfwTerminate();
    return 0;

    
}

