#include "InputHelper.h"
#include "ConversionTools.h"
#include <gl\freeglut.h>
#include <math.h>
#include <iostream>

InputSolver InputHelper::fSolver;

void InputHelper::OnMouseClick(int button, int state, int xPos, int yPos) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		InputHelper::setMouseButtonState(true);
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		InputHelper::setMouseButtonState(false);
		RefreshArray(fSolver.mySolver.sourceArray);
	}
}
void InputHelper::OnMouseDrag(int xPos, int yPos) {
	if (InputHelper::isMouseButtonDown()) {
		int arrayValue = ConversionTools::ConvertCoordToArray(xPos, yPos);

		if (arrayValue < ConversionTools::GetArrayLength() && arrayValue > 0) {
			fSolver.mySolver.sourceArray[arrayValue] = 1.0f;
		}
	}
}

void InputHelper::setMouseButtonState(bool value) {
	fSolver.mouseButtonDown = value;
}
bool const InputHelper::isMouseButtonDown() {
	return fSolver.mouseButtonDown;
}

void InputHelper::RefreshArray(float *sourceArray) {
	for (int i = 0; i < ConversionTools::GetResolution(); i++) {
		for (int j = 0; j < ConversionTools::GetResolution(); j++) {
			int arrayValue = ConversionTools::ConvertCoordToArray(i, j);
			sourceArray[arrayValue] = 0;
		}
	}
}

float InputHelper::ConvertWindowToGL(int number, bool isHeight) {
	float windowDimension;
	float newNumber = (float)number;
	if (isHeight) {
		windowDimension = (float)(glutGet(GLUT_WINDOW_HEIGHT) / 2);
		newNumber -= windowDimension;
		newNumber = newNumber / windowDimension;
		newNumber *= -1;
	}
	else {
		windowDimension = (float)(glutGet(GLUT_WINDOW_WIDTH) / 2);
		newNumber -= windowDimension;
		newNumber = newNumber / windowDimension;
	}
	return newNumber;
}

void InputHelper::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	glBegin(GL_POINTS);

	float* calculatedDensity = fSolver.mySolver.GetOutArray();

	for (int i = 0; i < ConversionTools::GetArrayLength(); i++) {

		Colour3 colourValue = DetermineColour(calculatedDensity[i]); //delete copy and assignment operators

		std::tuple<int, int> coords = ConversionTools::ConvertArraytoCoord(i);
		float x = ConvertWindowToGL(std::get<1>(coords), false);
		float y = ConvertWindowToGL(std::get<0>(coords), true);

		if (fSolver.mySolver.sourceArray[i] == 1.0f) {
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(x, y, 0.0f);
		}
		else {
			glColor3f(colourValue.getX(), colourValue.getY(), colourValue.getZ());
			glVertex3f(x, y, 0.0f);
		}
	}

	glEnd();
	glutSwapBuffers();
}

void InputHelper::Calculate() {
	fSolver.mySolver.VelocityStep(1.0f, 0.04f);
	fSolver.mySolver.DensityStep(fSolver.mySolver.sourceArray, 1.0f, 0.04f);
	Render();
}

Colour3 InputHelper::DetermineColour(float value)
{
	//make sure not 0 or will return error
	if (value == 0) {
		return Colour3(0.0f, 0.0f, 0.0f);
	}

	//make sure no negatives so log10 works correctly
	if (value < 0)
	{
		value *= -1;

		float logValue = log10(value);

		if (logValue < -40) {					//pruple
			return Colour3(0.3f, 0.0f, 0.3f);
		}
		else if (logValue < -30) {				//purple-blue
			return Colour3(0.5f, 0.0f, 0.8f);
		}
		else if (logValue < -20) {				//blue
			return Colour3(0.0f, 0.0f, 1.0f);
		}
		else if (logValue < -10) {				//blue-green
			return Colour3(0.0f, 0.5f, 0.5f);
		}
		else if (logValue < 0) {				//green
			return Colour3(0.0f, 1.0f, 0.0f);
		}
	}
	else {

		float logValue = log10(value);

		if (logValue < -40) {					//yellow
			return Colour3(1.0f, 1.0f, 0.0f);
		}
		else if (logValue < -30) {				//yellow-orange
			return Colour3(1.0f, 0.8f, 0.0f);
		}
		else if (logValue < -20) {				//orange
			return Colour3(1.0f, 0.5f, 0.0f);
		}
		else if (logValue < -10) {				//orange-red
			return Colour3(1.0f, 0.2f, 0.0f);
		}
		else if (logValue < 0) {				//red
			return Colour3(1.0f, 0.0f, 0.0f);
		}

	}
}