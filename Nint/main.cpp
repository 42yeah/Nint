// Nint.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "nint.h"

int main()
{
	FloatImage fi("examples/thinker.png");
	NormalMap nor(fi);
	FloatImage depth = nor.construct_depth_map();
	depth.write_to("depth.png");
	HeightMap height(depth);
	height.write_to("result.obj");
	FloatImage recovered = height.generate_normal_map();
	recovered.write_to("recovered.png");

	FloatImage shader(128, 128);
	for (int y = 0; y < shader.height; y++)
	{
		for (int x = 0; x < shader.width; x++)
		{
			shader.data[shader.offset(x, y, THROW) + 0] = ((float) x) / 255.0f;
			shader.data[shader.offset(x, y, THROW) + 1] = ((float) y) / 255.0f;
		}
	}
	shader.write_to("shader.png");

	return 0;
}
