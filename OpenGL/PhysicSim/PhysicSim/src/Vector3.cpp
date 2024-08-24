#include "Vector3.h"

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3 Vector3::scale(float s)
{
	return Vector3(
		this->x * s,
		this->y * s,
		this->z * s
	);
}

void Vector3::addVector(Vector3 v)
{
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;
}

Vector3 Vector3::addVectors(Vector3 v1, Vector3 v2)
{
	return Vector3(
		v1.x + v2.x,
		v1.y + v2.y,
		v1.z + v2.y
	);
}

Vector3 Vector3::subVectors(Vector3 v1, Vector3 v2)
{
	return Vector3(
		v1.x - v2.x,
		v1.y - v2.y,
		v1.z - v2.y
	);
}
