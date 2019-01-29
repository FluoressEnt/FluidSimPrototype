#pragma once

class Vector3 {
public:
	Vector3(float, float, float);
	float getX();
	float getY();
	float getZ();

private:
	float X;
	float Y;
	float Z;
};