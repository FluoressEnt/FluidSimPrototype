#include "ConversionTools.h"

int ConversionTools::ConvertCoordToArray(int xPos, int yPos) {
	//      IX(i,j) = ((i) + (N+2) * (j))
	int arrayValue = ((yPos)+(resolution + 2)*xPos);
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

int const ConversionTools::GetArrayLength() {
	return arrayLength;
}
int const ConversionTools::GetResolution() {
	return resolution;
}