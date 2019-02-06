#pragma once
#include <tuple>

class ConversionTools{
public:
	static int ConvertCoordToArray(int, int);
	static std::tuple<int, int> ConvertArraytoCoord(int);
	static std::tuple<float, float> ConvertCoordtoWindow(int, int);
	static int const GetArrayLength();
	static int const GetResolution();
private:
	static int const resolution = 400;
	static int const arrayLength = (resolution + 2)*(resolution + 2);
};