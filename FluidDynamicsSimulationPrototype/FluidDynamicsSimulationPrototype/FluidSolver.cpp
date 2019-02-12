#include "FluidSolver.h"
#include "ConversionTools.h"
//#include <math.h>
//#include <iostream>
//#include <fstream>

using namespace std;
//ofstream debugFile;


FluidSolver::FluidSolver(int n) :N(n)
{
	newVelocityArrayX = new float[ConversionTools::GetArrayLength()]();
	newVelocityArrayY = new float[ConversionTools::GetArrayLength()]();

	oldVelocityArrayX = new float[ConversionTools::GetArrayLength()]();
	oldVelocityArrayY = new float[ConversionTools::GetArrayLength()]();

	newDensityArray = new float[ConversionTools::GetArrayLength()]();
	oldDensityArray = new float[ConversionTools::GetArrayLength()]();

	sourceDens = new float[ConversionTools::GetArrayLength()];
	sourceVelX = new float[ConversionTools::GetArrayLength()];
	sourceVelY = new float[ConversionTools::GetArrayLength()];

	//debugFile.open("Debug.txt");
}
FluidSolver::~FluidSolver(void)
{
	//debugFile.close();
}

float* const FluidSolver::GetDensityArray() {
	return newDensityArray;
}
float* const FluidSolver::GetVelocityXArray() {
	return newVelocityArrayX;
}
float* const FluidSolver::GetVelocityYArray() {
	return newVelocityArrayY;
}

void FluidSolver::VelocityStep(float viscosity, float dt) {

	AddSource(newVelocityArrayX, sourceVelX, dt);
	AddSource(newVelocityArrayY, sourceVelY, dt);

	Swap(&oldVelocityArrayX, &newVelocityArrayX);
	Diffuse(1, newVelocityArrayX, oldVelocityArrayX, viscosity, dt);

	Swap(&oldVelocityArrayY, &newVelocityArrayY);
	Diffuse(2, newVelocityArrayY, oldVelocityArrayY, viscosity, dt);

	Projection(newVelocityArrayX, newVelocityArrayY, oldVelocityArrayX, oldVelocityArrayY);
	Swap(&oldVelocityArrayX, &newVelocityArrayX);
	Swap(&oldVelocityArrayY, &newVelocityArrayY);

	Advection(1, newVelocityArrayX, oldVelocityArrayX, oldVelocityArrayX, oldVelocityArrayY, dt);
	Advection(2, newVelocityArrayY, oldVelocityArrayY, oldVelocityArrayX, oldVelocityArrayY, dt);
	Projection(newVelocityArrayX, newVelocityArrayY, oldVelocityArrayX, oldVelocityArrayY);
}
void FluidSolver::DensityStep(float diff, float dt) {

	AddSource(newDensityArray, sourceDens, dt);
	Swap(&oldDensityArray, &newDensityArray);

	Diffuse(1, newDensityArray, oldDensityArray, diff, dt);
	Swap(&oldDensityArray, &newDensityArray);

	Advection(2, newDensityArray, oldDensityArray, newVelocityArrayX, newVelocityArrayY, dt);
}

void FluidSolver::Projection(float *newVelocityArrayX, float *newVelocityArrayY, float *oldVelocityArrayX, float *oldVelocityArrayY) {

	int i, j, GaussIterator;
	float h;
	h = 1.0f / N;

	//calculating the gradient field in y
	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			oldVelocityArrayY[ConversionTools::ConvertCoordToArray(i, j)] = -0.5f * h * (newVelocityArrayX[ConversionTools::ConvertCoordToArray(i + 1, j)] - newVelocityArrayX[ConversionTools::ConvertCoordToArray(i - 1, j)]
				+ newVelocityArrayY[ConversionTools::ConvertCoordToArray(i, j + 1)] - newVelocityArrayY[ConversionTools::ConvertCoordToArray(i, j - 1)]);
			oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j)] = 0;
		}
	}
	SetBoundary(N, 0, oldVelocityArrayY);
	SetBoundary(N, 0, oldVelocityArrayX);

	//itterating over the x component to get the height field
	for (GaussIterator = 0; GaussIterator < 20; GaussIterator++) {
		for (i = 1; i <= N; i++) {
			for (j = 1; j <= N; j++) {
				oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j)] = (oldVelocityArrayY[ConversionTools::ConvertCoordToArray(i, j)] + oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i - 1, j)] + oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i + 1, j)]
					+ oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j - 1)] + oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j + 1)]) / 4;
			}
		}
		SetBoundary(N, 0, oldVelocityArrayX);
	}

	//subtracting the the gradient of the height field from the velocity field to conserve mass
	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			newVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j)] -= 0.5f*(oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i + 1, j)] - oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i - 1, j)]) / h;
			newVelocityArrayY[ConversionTools::ConvertCoordToArray(i, j)] -= 0.5f*(oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j + 1)] - oldVelocityArrayX[ConversionTools::ConvertCoordToArray(i, j - 1)]) / h;
		}
	}
	SetBoundary(N, 1, newVelocityArrayX);
	SetBoundary(N, 2, newVelocityArrayY);
}
void FluidSolver::Advection(int b, float *newDensityArray, float *oldDensityArray, float *velocityArrayX, float *velocityArrayY, float dt) {

	int i, j, left, bottom, right, top;
	float x, y, distToRight, distToTop, distToLeft, distToBottom, ddistToTop;

	float dt0 = dt * N;
	for (i = 1; i < N; i++) {
		for (j = 1; j < N; j++) {
			x = i - dt0 * velocityArrayX[ConversionTools::ConvertCoordToArray(i, j)];
			y = j - dt0 * velocityArrayY[ConversionTools::ConvertCoordToArray(i, j)];

			//neighbourhood of previous position
			if (x < 0.5) x = 0.5f;
			if (x > N + 0.5) x = N + 0.5f;
			left = (int)x;
			right = left + 1;

			if (y < 0.5) y = 0.5;
			if (y > N + 0.5) y = N + 0.5f;
			bottom = (int)y;
			top = bottom + 1;

			//interpolation part
			distToLeft = x - left;
			distToRight = 1 - distToLeft;
			distToBottom = y - bottom;
			distToTop = 1 - distToBottom;

			newDensityArray[ConversionTools::ConvertCoordToArray(i, j)] = distToRight * (distToTop*oldDensityArray[ConversionTools::ConvertCoordToArray(left, bottom)] + distToBottom * oldDensityArray[ConversionTools::ConvertCoordToArray(left, top)]) +
				distToLeft * (distToTop*oldDensityArray[ConversionTools::ConvertCoordToArray(right, bottom)] + distToBottom * oldDensityArray[ConversionTools::ConvertCoordToArray(right, top)]);
		}
	}
	SetBoundary(N, b, newDensityArray);

}
void FluidSolver::Diffuse(int b, float *newDensityArray, float *oldDensityArray, float diff, float dt) {

	int i, j, GaussIterator;
	float a = dt * diff * N * N; //conservation of diffusivity of the entire grid(?)

	for (GaussIterator = 0; GaussIterator < 20; GaussIterator++) {
		for (i = 1; i < N; i++) {
			for (j = 1; j < N; j++) {
				newDensityArray[ConversionTools::ConvertCoordToArray(i, j)] = (oldDensityArray[ConversionTools::ConvertCoordToArray(i, j)] + a * (newDensityArray[ConversionTools::ConvertCoordToArray(i - 1, j)] + newDensityArray[ConversionTools::ConvertCoordToArray(i + 1, j)] +
					newDensityArray[ConversionTools::ConvertCoordToArray(i, j - 1)] + newDensityArray[ConversionTools::ConvertCoordToArray(i, j + 1)])) / (1 + 4 * a);
			}
		}
		SetBoundary(N, b, newDensityArray);
	}
}

void FluidSolver::AddSource(float *newDensityArray, float *sourceArray, float dt) {

	int i, size = (N + 2)*(N + 2);
	for (i = 0; i < size; i++) {
		float temp;
		temp = (dt * sourceArray[i]);
		newDensityArray[i] += temp;
	}
}

void FluidSolver::Swap(float** arrayOne, float** arrayTwo) {
	float* temp = *arrayOne;
	*arrayOne = *arrayTwo;
	*arrayTwo = temp;
}
void FluidSolver::SetBoundary(int resolution, int b, float* boundaryArray) {
	int i;
	for (i = 1; i <= resolution; i++) {
		boundaryArray[ConversionTools::ConvertCoordToArray(0, i)] = b == 1 ? -boundaryArray[ConversionTools::ConvertCoordToArray(1, i)] : boundaryArray[ConversionTools::ConvertCoordToArray(1, i)];
		boundaryArray[ConversionTools::ConvertCoordToArray(resolution + 1, i)] = b == 1 ? -boundaryArray[ConversionTools::ConvertCoordToArray(resolution, i)] : boundaryArray[ConversionTools::ConvertCoordToArray(resolution, i)];
		boundaryArray[ConversionTools::ConvertCoordToArray(i, 0)] = b == 2 ? -boundaryArray[ConversionTools::ConvertCoordToArray(i, 1)] : boundaryArray[ConversionTools::ConvertCoordToArray(i, 1)];
		boundaryArray[ConversionTools::ConvertCoordToArray(i, resolution + 1)] = b == 2 ? -boundaryArray[ConversionTools::ConvertCoordToArray(i, resolution)] : boundaryArray[ConversionTools::ConvertCoordToArray(i, resolution)];
	}
	boundaryArray[ConversionTools::ConvertCoordToArray(0, 0)] = 0.5f *(boundaryArray[ConversionTools::ConvertCoordToArray(1, 0)] + boundaryArray[ConversionTools::ConvertCoordToArray(0, 1)]);
	boundaryArray[ConversionTools::ConvertCoordToArray(0, resolution + 1)] = 0.5f *(boundaryArray[ConversionTools::ConvertCoordToArray(1, resolution + 1)] + boundaryArray[ConversionTools::ConvertCoordToArray(0, resolution)]);
	boundaryArray[ConversionTools::ConvertCoordToArray(resolution + 1, 0)] = 0.5f *(boundaryArray[ConversionTools::ConvertCoordToArray(resolution, 0)] + boundaryArray[ConversionTools::ConvertCoordToArray(resolution + 1, 1)]);
	boundaryArray[ConversionTools::ConvertCoordToArray(resolution + 1, resolution + 1)] = 0.5f *(boundaryArray[ConversionTools::ConvertCoordToArray(resolution, resolution + 1)] + boundaryArray[ConversionTools::ConvertCoordToArray(resolution + 1, resolution)]);
}

void FluidSolver::Refresh() {
	//clear old arrays
	delete newVelocityArrayX;
	delete newVelocityArrayY;

	delete oldVelocityArrayX;
	delete oldVelocityArrayY;

	delete newDensityArray;
	delete oldDensityArray;

	delete sourceDens;
	delete sourceVelX;
	delete sourceVelY;

	//create new arrays initialised at 0
	newVelocityArrayX = new float[ConversionTools::GetArrayLength()]();
	newVelocityArrayY = new float[ConversionTools::GetArrayLength()]();

	oldVelocityArrayX = new float[ConversionTools::GetArrayLength()]();
	oldVelocityArrayY = new float[ConversionTools::GetArrayLength()]();

	newDensityArray = new float[ConversionTools::GetArrayLength()]();
	oldDensityArray = new float[ConversionTools::GetArrayLength()]();

	sourceDens = new float[ConversionTools::GetArrayLength()];
	sourceVelX = new float[ConversionTools::GetArrayLength()];
	sourceVelY = new float[ConversionTools::GetArrayLength()];
}
