#pragma once

class FluidSolver {
public:
	FluidSolver(int);
	~FluidSolver(void);

	float* sourceDens;
	float* sourceVelX;
	float* sourceVelY;

	void VelocityStep(float, float);
	void DensityStep(float, float);
	float* const GetDensityArray();
	float* const GetVelocityXArray();
	float* const GetVelocityYArray();
	void Refresh();

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
	void Swap(float**, float**);
	void Swap2(float*, float*);
};