#pragma once
#include <GL/glew.h>
#include "Vector3.h"
#include <math.h>

struct color {
	float R;
	float G;
	float B;
	float T;
};
class Particle
{


public:
	Particle();
	Particle(Vector3 pos, Vector3 vel, float radius, color color = {1.0, 0.0, 0.0, 1.0});

	Vector3 pos;
	Vector3 vel;
	float radius;

	

	color rgbt;

	const void Draw();
	void Simulate(float dt);
	void CollideWithBoundry();
};
