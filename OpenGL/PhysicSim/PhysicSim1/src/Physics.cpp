#include "Physics.h"


void Physics::SetupScene(std::vector<Particle> &parts)
{
	for (int i = 0; i < 10; i++)
	{
		parts.push_back(Particle(Vector2(10.0, 10.0), Vector2(i * 1, 100.0), 10));
	}
	
}

std::vector<Particle> Physics::particles;

const float Physics::restitution = 1;

