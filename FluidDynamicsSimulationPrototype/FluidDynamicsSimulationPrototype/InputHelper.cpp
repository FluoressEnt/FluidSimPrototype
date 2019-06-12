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

float totalTime;
int itteration;

///A function that catches specific key presses
void InputHelper::OnKeyDown(unsigned char key, int x, int y) {
	switch (key) {
	case 32:									//spacebar - swap visualisation to render
		diffuseDisplay = !diffuseDisplay;
	
	case 82:									//R - refreshes all arrays in simulation
		fSolver.mySolver.Refresh();

	case 114:									//r - refreshes all arrays in simulation
		fSolver.mySolver.Refresh();
	
	case 116:									//t - calculates itterations and timestep and prints to debug console
			cout << " itterations: " << itteration << " mean timestep: " << totalTime / itteration << endl;
	}
	

}
///A function that updates the X&Y position of the mouse or refreshes the input arrays
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
			RefreshArray(fSolver.mySolver.sDens);
		else {
			RefreshArray(fSolver.mySolver.sVelX);
			RefreshArray(fSolver.mySolver.sVelY);
		}
	}
}
///A function that refreshes the input arrays to 0, updates the X&Y position of the mouse
///Then assigns that frames values for the inputof either density or velocity depenant on the current visualisation selected
void InputHelper::OnMouseDrag(int xPos, int yPos) {
	if (InputHelper::isMouseButtonDown() && xPos > 0 && xPos < ConversionTools::GetResolution()) {

		if (diffuseDisplay)
			RefreshArray(fSolver.mySolver.sDens);
		else {
			RefreshArray(fSolver.mySolver.sVelX);
			RefreshArray(fSolver.mySolver.sVelY);
		}

		mouseX = xPos;
		mouseY = yPos;

		int arrayValue = ConversionTools::ConvertCoordToArray(xPos, yPos);

		if (arrayValue < ConversionTools::GetArrayLength() && arrayValue > 0) {
			if (diffuseDisplay)
				fSolver.mySolver.sDens[arrayValue] = 1.0f;
			else {
				fSolver.mySolver.sVelX[arrayValue] += mouseX - oldMouseX;
				fSolver.mySolver.sVelY[arrayValue] += oldMouseY - mouseY;

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

///function to refresh the source array
void InputHelper::RefreshArray(float *sourceArray) {
	for (int i = 0; i < ConversionTools::GetResolution(); i++) {
		for (int j = 0; j < ConversionTools::GetResolution(); j++) {
			int arrayValue = ConversionTools::ConvertCoordToArray(i, j);
			sourceArray[arrayValue] = 0;
		}
	}
}
///a function to shift the origin to centre of screen
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

///A function that renders either the representation of diffusion or velocity field 
void InputHelper::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	//toggle display between diffuse and vector field

	if (diffuseDisplay) {
		glBegin(GL_POINTS);
		//retreive the solver's most recent density array
		float* calculatedDensity = fSolver.mySolver.GetDensityArray();

		for (int i = 0; i < ConversionTools::GetArrayLength(); i++) {

			//determine point's colour based on a clamped scale
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

		//retreive solver's most recent velocity arrays
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

///A function that calls the solver's calculate function and measures the time taken to perform one itteration
void InputHelper::Calculate() {

	auto start = high_resolution_clock::now();

	fSolver.mySolver.VelocityStep(1.0f, dt);
	fSolver.mySolver.DensityStep(1.0f, dt);

	auto end = high_resolution_clock::now();

	duration<double> timeSpan = duration_cast<duration<double>>(end - start);
	dt = timeSpan.count();

	itteration++;
	totalTime += dt;

	glutPostRedisplay();
}

///A function that determines the colour of a value using a clamped scale
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
		if (logValue < -35) {					//yellow
			return Colour3(1.0f, 0.9f, 0.0f);
		}
		else if (logValue < -30) {				//yellow-orange
			return Colour3(1.0f, 0.8f, 0.0f);
		}
		else if (logValue < -25) {				//orange
			return Colour3(1.0f, 0.7f, 0.0f);
		}
		else if (logValue < -20) {				//orange
			return Colour3(1.0f, 0.6f, 0.0f);
		}
		else if (logValue < -15) {				//orange-red
			return Colour3(1.0f, 0.5f, 0.0f);
		}
		else if (logValue < -10) {				//orange-red
			return Colour3(1.0f, 0.4f, 0.0f);
		}
		else if (logValue < -7) {				//red
			return Colour3(1.0f, 0.3f, 0.0f);
		}
		else if (logValue < -5) {				//red
			return Colour3(1.0f, 0.2f, 0.0f);
		}
		else if (logValue < -2) {				//red
			return Colour3(1.0f, 0.1f, 0.0f);
		}
		else if (logValue < 0) {				//red
			return Colour3(1.0f, 0.0f, 0.0f);
		}

	}
	return Colour3(0.0f, 0.0f, 0.0f);
}