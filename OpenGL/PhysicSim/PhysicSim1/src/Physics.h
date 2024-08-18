#pragma once
#include "Vector2.h"
#include "Particle.h"

class Physics
{
public:
	static std::vector<Particle> particles;
	static void SetupScene(std::vector<Particle>& parts);
	static const float restitution;
};

/*
namespace Physics
{
	std::vector<Particle> particles;
	void SetupScene(std::vector<Particle>& parts);

}
*/