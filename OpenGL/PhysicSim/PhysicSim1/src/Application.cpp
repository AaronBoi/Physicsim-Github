#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Physics.h"
#include "Vector2.h"
#include "Particle.h"
#include "Core.h"
#include "shader.h"

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
    
    Core::EnableDebug();
    Physics::particles.reserve(50);
    //particles.resize(1);

    Physics::SetupScene(Physics::particles);
   
    float quads[] = {
        -0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
        0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
        0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
        -0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 1.0,

    };
    Shader shader("res/shaders/Basic.shader");


    unsigned int vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(Core::circleData), Core::circleData, GL_STATIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quads), quads, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), nullptr);
    
    glEnableVertexAttribArray(0);

    



    //std::cout << Core::simWidth << std::endl;
    glDrawArrays(GL_QUADS, 0, 20);
    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        //glClear(GL_COLOR_BUFFER_BIT);
        for (int i = 0; i < Physics::particles.size(); i++)
        {

            //Physics::particles[i].Simulate(Core::dt);
            //Physics::particles[i].Draw();

        }
        
        //glDrawArrays(GL_LINE_LOOP, 0, 10 * sizeof(float));
        
       
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

