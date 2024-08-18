#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <Particle.h>

class Core
{   

public:
    static const float dt;
    
    
    struct window {
         static int height;
         static int width;
    };

    static void window_size_callback(GLFWwindow* window, int width, int height);

    static float transX(float x);
    static float transY(float y);

    static float simHeight;
    static float simWidth;

    static float circleData[10];
    static float quadData[10];
    
};