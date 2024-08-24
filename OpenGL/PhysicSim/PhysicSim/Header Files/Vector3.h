#pragma once
#include <vector>
class Vector3
{
public:

	Vector3(float x = 0.0, float y = 0.0, float z = 0.0);
	float x;
	float y;
	float z;


	Vector3 scale(float s);
	void addVector(Vector3 v);
	static Vector3 addVectors(Vector3 v1, Vector3 v2);
	static Vector3 subVectors(Vector3 v1, Vector3 v2);

};