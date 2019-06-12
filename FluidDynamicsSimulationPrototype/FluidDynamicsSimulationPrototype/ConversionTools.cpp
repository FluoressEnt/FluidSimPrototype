#include "ConversionTools.h"


///function that converts 2d coordinates (i,j) to a 1d array index
int ConversionTools::ConvertCoordToArray(int xPos, int yPos) {
	int arrayValue = ((xPos)+(resolution + 2)*yPos);
	return arrayValue;
}
///function that converts 1d array index to 2d coordinates (i,j)
std::tuple<int, int> ConversionTools::ConvertArraytoCoord(int arrayValue) {
	int xPos, yPos;
	xPos = arrayValue % (resolution + 2);
	yPos = (arrayValue - xPos) / (resolution + 2);
	std::tuple<int, int> coordinates(xPos, yPos);
	return coordinates;
}

///function that converts 2d coordinates (i,j) to the window space coordinates (i,j)
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

///function that returns the array length
int const ConversionTools::GetArrayLength() {
	return arrayLength;
}

///function that returns the resolution of the array
int const ConversionTools::GetResolution() {
	return resolution;
}