#include "Physics.h"

void Physics::SetupScene()
{
	for (int i = 0; i < 10; i++)
	{
		particles.push_back(Particle(Vector3(1.0 * i, 10.0), Vector3(i * 1.0, 100.0), 10));
	}

}

std::vector<Particle> Physics::particles;

const float Physics::dt = 1.0 / 60.0;

const float Physics::restitution = 1;
