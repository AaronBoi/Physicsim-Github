#include "Particle.h"
#include <iostream>
#include "Physics.h"
#include "Renderer.h"

Particle::Particle()
{
    this->pos = Vector3();
    this->vel = Vector3();
    this->radius = 20;
    this->rgbt = { 1.0, 0.0, 0.0, 1.0 };
}

Particle::Particle(Vector3 pos, Vector3 vel, float radius, color color)
{
    this->pos = pos;
    this->vel = vel;
    this->radius = radius;
    this->rgbt = color;
}

//float Particle::color::R;

const void Particle::Draw()
{
    std::vector<float> vert;
    std::vector<float> lastPoint = {};

    static const int circle_points = 20;
    static const float angle = 2.0f * 3.1416f / circle_points;

   
    double angle1 = 0.0;

    for (int i = 0; i < circle_points; i++)
    {
        
        angle1 += angle;
        std::vector<float> point = {
            Renderer::transX(pos.x + radius * cos(angle1)), 
            Renderer::transY(pos.y + radius * sin(angle1)), 
            pos.z,
            rgbt.R,
            rgbt.G,
            rgbt.B,
            rgbt.T,
            
        };
        vert.insert(vert.end(), point.begin(), point.end());
        if (!lastPoint.empty())
        {
            vert.insert(vert.end(), lastPoint.begin(), lastPoint.end());
        }
        lastPoint = point;
    }

    vert.insert(vert.end(), lastPoint.begin(), lastPoint.end());
    Renderer::pushLines(vert);

}

void Particle::Simulate(float dt)
{
    this->pos.addVector(this->vel.scale(dt));
    
    this->CollideWithBoundry();
}

void Particle::CollideWithBoundry()
{

    if (this->pos.x <= -Renderer::simWidth + this->radius)
    {
        this->pos.x = -Renderer::simWidth + this->radius;
        this->vel.x *= -Physics::restitution;
    }

    if (this->pos.x >= Renderer::simWidth - this->radius)
    {
        this->pos.x = Renderer::simWidth - this->radius;
        this->vel.x *= -Physics::restitution;
    }
    if (this->pos.y <= -Renderer::simHeight + this->radius)
    {
        this->pos.y = -Renderer::simHeight + this->radius;
        this->vel.y *= -Physics::restitution;
    }
    if (this->pos.y >= Renderer::simHeight - this->radius)
    {
        this->pos.y = Renderer::simHeight - this->radius;
        this->vel.y *= -Physics::restitution;
    }

}
