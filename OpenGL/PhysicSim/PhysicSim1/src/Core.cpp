#include "Core.h"


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