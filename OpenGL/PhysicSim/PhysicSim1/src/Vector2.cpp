#include "Vector2.h"

Vector2::Vector2()
	:x(0.0), y(0.0)
{
	
}

Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

void Vector2::scale(float s)
{
	this->x *= s;
	this->y *= s;
}

void Vector2::addVector(Vector2 v, float s)
{
	this->x += v.x * s;
	this->y += v.y * s;
}