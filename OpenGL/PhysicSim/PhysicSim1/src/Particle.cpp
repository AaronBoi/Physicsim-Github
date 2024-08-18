#include "Particle.h"
#include <iostream>
#include "Core.h"
#include "Physics.h"

Particle::Particle()
{
    this->pos = Vector2();
    this->vel = Vector2();
    this->radius = 20;
}

Particle::Particle(Vector2 pos, Vector2 vel, float radius)
{
	this->pos = pos;
    this->vel = vel;
    this->radius = radius;
}



void Particle::Draw()
{
    //std::cout << Core::transX(radius) << std::endl;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(Core::transX(pos.x), Core::transY(pos.y), 0.0f);
    static const int circle_points = 100;
    static const float angle = 2.0f * 3.1416f / circle_points;

    // this code (mostly) copied from question:
    glBegin(GL_POLYGON);
    double angle1 = 0.0;
    //glVertex2d(Core::transX(radius) * cos(0.0), Core::transY(radius) * sin(0.0));
 
    for (int i = 0; i < circle_points; i++)
    {
        glVertex2d(Core::transX(radius) * cos(angle1), Core::transY(radius) * sin(angle1));
        angle1 += angle;
    }
    glEnd();
    glPopMatrix();
	
}

void Particle::Simulate(float dt)
{
    this->pos.addVector(this->vel, dt);
    this->CollideWithBoundry();
}

void Particle::CollideWithBoundry()
{
    
    if (this->pos.x <= -Core::simWidth + this->radius)
    {
        this->pos.x = -Core::simWidth + this->radius;
        this->vel.x *= -Physics::restitution;
    }
    
    if (this->pos.x >= Core::simWidth - this->radius)
    {
        this->pos.x = Core::simWidth - this->radius;
        this->vel.x *= -Physics::restitution;
    }
    if (this->pos.y <= -Core::simHeight + this->radius)
    {
        this->pos.y = -Core::simHeight + this->radius;
        this->vel.y *= -Physics::restitution;
    }
    if (this->pos.y >= Core::simHeight - this->radius)
    {
        this->pos.y = Core::simHeight - this->radius;
        this->vel.y *= -Physics::restitution;
    }
    
}
