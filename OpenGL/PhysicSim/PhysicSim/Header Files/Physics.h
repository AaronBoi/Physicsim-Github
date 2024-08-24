#pragma once
#include "Vector3.h"
#include "Particle.h"

class Physics
{
	public:
		static std::vector<Particle> particles;
		static void SetupScene();
		static const float dt;
		static const float restitution;

};