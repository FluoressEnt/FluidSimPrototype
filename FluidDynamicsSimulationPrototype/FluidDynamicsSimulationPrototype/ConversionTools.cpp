#include "ConversionTools.h"

int ConversionTools::ConvertCoordToArray(int xPos, int yPos) {
	//      IX(i,j) = ((i) + (N+2) * (j))
	int arrayValue = ((xPos)+(resolution + 2)*yPos);
	return arrayValue;
}
std::tuple<int, int> ConversionTools::ConvertArraytoCoord(int arrayValue) {
	int xPos, yPos;
	xPos = arrayValue % (resolution + 2);
	yPos = (arrayValue - xPos) / (resolution + 2);
	//cout << "xPos: " << xPos << " yPos: " << yPos << endl;
	std::tuple<int, int> coordinates(xPos, yPos);
	return coordinates;
}

std::tuple<float, float> ConversionTools::ConvertCoordtoWindow(int xPos, int yPos) {
	float newX = xPos;
	float newY = yPos;

	if (xPos < (resolution / 2)) {
		newX = (1 - (newX / (resolution / 2)))*-1;
	}
	else {
		newX = (newX - (resolution / 2)) / (resolution / 2);
	}

	if (yPos < (resolution / 2)) {
		newY = 1 - (newY / (resolution / 2));
	}
	else {
		newY = ((newY - (resolution / 2)) / (resolution / 2))*-1;
	}

	std::tuple<float, float> WindowSpace(newX, newY);
	return	WindowSpace;
}

int const ConversionTools::GetArrayLength() {
	return arrayLength;
}
int const ConversionTools::GetResolution() {
	return resolution;
}