#pragma once

class FluidSolver {
public:
	FluidSolver(int);
	~FluidSolver(void);

	float* sDens;
	float* sVelX;
	float* sVelY;

	void VelocityStep(float, float);
	void DensityStep(float, float);
	float* const GetDensityArray();
	float* const GetVelocityXArray();
	float* const GetVelocityYArray();
	void Refresh();

private:
	float* newVelX;
	float* newVelY;
	float* oldVelX;
	float* oldVelY;
	float* newDens;
	float* oldDens;
	int N;

	void Projection(float*, float*, float*, float*);
	void Advection(int, float *, float *, float *, float *, float);
	void Diffuse(int, float *, float *, float, float);
	void AddSource(float *, float *, float);
	void SetBoundary(int, int, float*);
	void Swap(float**, float**);
};