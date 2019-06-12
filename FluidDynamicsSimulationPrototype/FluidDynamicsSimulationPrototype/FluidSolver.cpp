#include "FluidSolver.h"
#include "ConversionTools.h"

using namespace std;


FluidSolver::FluidSolver(int n) :N(n)
{
	newVelX = new float[ConversionTools::GetArrayLength()]();
	newVelY = new float[ConversionTools::GetArrayLength()]();

	oldVelX = new float[ConversionTools::GetArrayLength()]();
	oldVelY = new float[ConversionTools::GetArrayLength()]();

	newDens = new float[ConversionTools::GetArrayLength()]();
	oldDens = new float[ConversionTools::GetArrayLength()]();

	sDens = new float[ConversionTools::GetArrayLength()];
	sVelX = new float[ConversionTools::GetArrayLength()];
	sVelY = new float[ConversionTools::GetArrayLength()];
}
FluidSolver::~FluidSolver(void)
{
}

float* const FluidSolver::GetDensityArray() {
	return newDens;
}
float* const FluidSolver::GetVelocityXArray() {
	return newVelX;
}
float* const FluidSolver::GetVelocityYArray() {
	return newVelY;
}

void FluidSolver::VelocityStep(float viscosity, float dt) {

	AddSource(newVelX, sVelX, dt);
	AddSource(newVelY, sVelY, dt);

	Swap(&oldVelX, &newVelX);
	Diffuse(1, newVelX, oldVelX, viscosity, dt);

	Swap(&oldVelY, &newVelY);
	Diffuse(2, newVelY, oldVelY, viscosity, dt);

	Projection(newVelX, newVelY, oldVelX, oldVelY);
	Swap(&oldVelX, &newVelX);
	Swap(&oldVelY, &newVelY);

	Advection(1, newVelX, oldVelX, oldVelX, oldVelY, dt);
	Advection(2, newVelY, oldVelY, oldVelX, oldVelY, dt);
	Projection(newVelX, newVelY, oldVelX, oldVelY);
}
void FluidSolver::DensityStep(float diff, float dt) {

	AddSource(newDens, sDens, dt);
	Swap(&oldDens, &newDens);

	Diffuse(1, newDens, oldDens, diff, dt);
	Swap(&oldDens, &newDens);

	Advection(2, newDens, oldDens, newVelX, newVelY, dt);
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
	delete newVelX;
	delete newVelY;

	delete oldVelX;
	delete oldVelY;

	delete newDens;
	delete oldDens;

	delete sDens;
	delete sVelX;
	delete sVelY;

	//create new arrays initialised at 0
	newVelX = new float[ConversionTools::GetArrayLength()]();
	newVelY = new float[ConversionTools::GetArrayLength()]();

	oldVelX = new float[ConversionTools::GetArrayLength()]();
	oldVelY = new float[ConversionTools::GetArrayLength()]();

	newDens = new float[ConversionTools::GetArrayLength()]();
	oldDens = new float[ConversionTools::GetArrayLength()]();

	sDens = new float[ConversionTools::GetArrayLength()];
	sVelX = new float[ConversionTools::GetArrayLength()];
	sVelY = new float[ConversionTools::GetArrayLength()];
}
