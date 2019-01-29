#pragma once
#include "FluidSolver.h"
#include "ConversionTools.h"
#include "Vector3.h"

typedef struct _InputSolver {
	FluidSolver mySolver = FluidSolver(ConversionTools::GetResolution());
	bool mouseButtonDown = false;
}InputSolver;

class InputHelper {
public:
	static InputSolver fSolver;
	static void OnMouseClick(int, int, int, int);
	static void OnMouseDrag(int, int);
	static void Render();
	static void Calculate();

	static void RefreshArray(float *sourceArray);

private:
	static float ConvertWindowToGL(int, bool);
	static void setMouseButtonState(bool);
	static bool const isMouseButtonDown();
	static Vector3 DetermineColour(float);
};