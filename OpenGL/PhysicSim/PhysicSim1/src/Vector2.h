#pragma once
#include <vector>
class Vector2
{
public:
	Vector2();
	Vector2(float x, float y);
	float x;
	float y;


	void scale(float s);
	void addVector(Vector2 v, float s = 1.0);

};