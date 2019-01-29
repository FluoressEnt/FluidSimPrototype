#pragma once
#include <tuple>

class ConversionTools{
public:
	static int ConvertCoordToArray(int xPos, int yPos);
	static std::tuple<int, int> ConvertArraytoCoord(int arrayValue);
	static int const GetArrayLength();
	static int const GetResolution();
private:
	static int const resolution = 400;
	static int const arrayLength = (resolution + 2)*(resolution + 2);
};