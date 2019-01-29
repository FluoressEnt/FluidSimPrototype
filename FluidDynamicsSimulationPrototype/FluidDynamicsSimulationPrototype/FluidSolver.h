#pragma once

class FluidSolver {
public:
	FluidSolver(int);
	~FluidSolver(void);

	float* sourceArray;

	void VelocityStep(float, float);
	void DensityStep(float*, float, float);
	float* const GetOutArray();

private:
	float* newVelocityArrayX;
	float* newVelocityArrayY;
	float* oldVelocityArrayX;
	float* oldVelocityArrayY;
	float* newDensityArray;
	float* oldDensityArray;
	int N;

	void Projection(float*, float*, float*, float*);
	void Advection(int, float *, float *, float *, float *, float);
	void Diffuse(int, float *, float *, float, float);
	void AddSource(float *, float *, float);
	void SetBoundary(int, int, float*);
	void Swap(float*, float*);
};