#include "InputHelper.h"
#include "ConversionTools.h"
#include <gl\freeglut.h>
#include <chrono> 
#include <iostream>

using namespace std;
using namespace std::chrono;

InputSolver InputHelper::fSolver;
float dt = 0.04f;
int mouseX;
int mouseY;
int oldMouseX;
int oldMouseY;
bool diffuseDisplay = true;

void InputHelper::OnKeyDown(unsigned char key, int x, int y) {
	switch (key) {
	case 32:
		diffuseDisplay = !diffuseDisplay;
	
	case 82:
		fSolver.mySolver.Refresh();

	case 114:
		fSolver.mySolver.Refresh();
	}

}

void InputHelper::OnMouseClick(int button, int state, int xPos, int yPos) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		InputHelper::setMouseButtonState(true);
		mouseX = xPos;
		mouseY = yPos;

		oldMouseX = mouseX;
		oldMouseY = mouseY;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		InputHelper::setMouseButtonState(false);
		if (diffuseDisplay)
			RefreshArray(fSolver.mySolver.sourceDens);
		else {
			RefreshArray(fSolver.mySolver.sourceVelX);
			RefreshArray(fSolver.mySolver.sourceVelY);
		}
	}
}
void InputHelper::OnMouseDrag(int xPos, int yPos) {
	if (InputHelper::isMouseButtonDown() && xPos > 0 && xPos < ConversionTools::GetResolution()) {

		if (diffuseDisplay)
			RefreshArray(fSolver.mySolver.sourceDens);
		else {
			RefreshArray(fSolver.mySolver.sourceVelX);
			RefreshArray(fSolver.mySolver.sourceVelY);
		}

		mouseX = xPos;
		mouseY = yPos;

		int arrayValue = ConversionTools::ConvertCoordToArray(xPos, yPos);

		if (arrayValue < ConversionTools::GetArrayLength() && arrayValue > 0) {
			if (diffuseDisplay)
				fSolver.mySolver.sourceDens[arrayValue] = 1.0f;
			else {
				fSolver.mySolver.sourceVelX[arrayValue] += mouseX - oldMouseX;
				fSolver.mySolver.sourceVelY[arrayValue] += oldMouseY - mouseY;

			}
		}

		oldMouseX = mouseX;
		oldMouseY = mouseY;
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
//shifts origin to centre of screen
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

	//toggle display between diffuse and vector field

	if (diffuseDisplay) {
		glBegin(GL_POINTS);
		float* calculatedDensity = fSolver.mySolver.GetDensityArray();

		for (int i = 0; i < ConversionTools::GetArrayLength(); i++) {

			Colour3 colourValue = DetermineColour(calculatedDensity[i]);

			tuple<int, int> coords = ConversionTools::ConvertArraytoCoord(i);
			float x = ConvertWindowToGL(std::get<0>(coords), false);
			float y = ConvertWindowToGL(std::get<1>(coords), true);

			glColor3f(colourValue.getX(), colourValue.getY(), colourValue.getZ());
			glVertex3f(x, y, 0.0f);

		}
		glEnd();
		glutSwapBuffers();
	}

	else {
		glBegin(GL_LINES);
		glLineWidth(1.0f);
		glColor3f(1.0f, 1.0f, 1.0f);

		float* calculatedVelocityX = fSolver.mySolver.GetVelocityXArray();
		float* calculatedVelocityY = fSolver.mySolver.GetVelocityYArray();

		for (int i = 5; i <= ConversionTools::GetResolution(); i = i + 10) {
			for (int j = 5; j <= ConversionTools::GetResolution(); j = j + 10) {

				int arrayPos = ConversionTools::ConvertCoordToArray(i, j);
				float velX = calculatedVelocityX[arrayPos];
				float velY = calculatedVelocityY[arrayPos];

				float magnitude = sqrt(velX*velX + velY * velY);

				if (magnitude != 0) {
					//find unit components
					float unitX = velX / magnitude;
					float unitY = velY / magnitude;

					//scale components with log and scalar to standardise the size & make visible
					float scale = log(magnitude*1e20 + 1.0);
					float newX = unitX * scale;
					float newY = unitY * scale;

					//creating coordinates in window space where line is drawn around the origin i,j
					tuple<float, float> startWindowPos = ConversionTools::ConvertCoordtoWindow(i - newX / 2, j - newY / 2);
					tuple<float, float> endWindowPos = ConversionTools::ConvertCoordtoWindow(i + newX / 2, j + newY / 2);
					//cout << "x: " << newI-i << " y: " << newJ-j << endl;

					//start of line
					glVertex2f(get<0>(startWindowPos), get<1>(startWindowPos));
					//end of line
					glVertex2f(get<0>(endWindowPos), get<1>(endWindowPos));
				}
			}
		}
		glEnd();
		glutSwapBuffers();
	}
}

void InputHelper::Calculate() {

	auto start = high_resolution_clock::now();

	fSolver.mySolver.VelocityStep(1.0f, dt);
	fSolver.mySolver.DensityStep(1.0f, dt);
	glutPostRedisplay();

	auto end = high_resolution_clock::now();

	duration<double> timeSpan = duration_cast<duration<double>>(end - start);
	dt = timeSpan.count();
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