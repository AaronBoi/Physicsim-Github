#pragma once
#include <GL/glew.h>
#include "Vector2.h"
#include <math.h>


class Particle
{
	

public:
	Particle();
	Particle(Vector2 pos, Vector2 vel, float radius);
	Vector2 pos;
	Vector2 vel;
	float radius;
	void Draw();
	void Simulate(float dt);
	void CollideWithBoundry();
};
