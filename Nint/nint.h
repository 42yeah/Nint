#pragma once

#include <iostream>

enum OffsetBehavior
{
	THROW, CLAMP_TO_EDGE, REPEAT 
};

//
// An image in float format.
// RGBA; Channel A is ignored.
//
class FloatImage
{
public:
	FloatImage(const std::string& path);

	FloatImage(int width, int height);

	void write_to(const std::string &path);

	int offset(int x, int y, OffsetBehavior behavior);

	int width, height;
	std::string path;
	std::unique_ptr<float[]> data;
};

class NormalMap
{
public:
	NormalMap(FloatImage& fi);

	FloatImage construct_depth_map() const;

	FloatImage& fi;
};

class HeightMap
{
public:
	HeightMap(FloatImage& fi);

	void write_to(const std::string& path);

	FloatImage generate_normal_map() const;

	FloatImage& fi;
};
