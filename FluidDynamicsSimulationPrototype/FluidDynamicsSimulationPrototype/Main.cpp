#include <Windows.h>
#include <gl/freeglut.h>
#include "ConversionTools.h"
#include "FluidSolver.h"
#include "InputHelper.h"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {

	//setup window
	glutInit(&argc, argv);
	glutInitWindowSize(ConversionTools::GetResolution(), ConversionTools::GetResolution());
	glutInitWindowPosition(800, 100);
	glutCreateWindow("Fluid Dynamics Simulation Prototype");

	//register callback functions
	glutMouseFunc(InputHelper::OnMouseClick);
	glutMotionFunc(InputHelper::OnMouseDrag);
	glutDisplayFunc(InputHelper::Render);
	glutIdleFunc(InputHelper::Calculate);

	//initialise GL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPointSize(2.0f);

	// Enter the event-processing loop
	glutMainLoop();
	return 0;

	//TODO: create boundary condition conversion from source array to used arrays - check if this is appropriate
	//TODO: implement appropriate dt calculation from chrono high res clock
	//TODO: implement more informative visual feedback display
}