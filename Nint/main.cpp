#include <iostream>
#include "nint.h"

//
// What this code do:
// 1. recover depth from blurball.png
// 2. write it to depth.png
// 3. recover mesh from depth
// 4. create normap map from depth so we can compare between the results
//
int main()
{
	FloatImage fi("examples/blurball.png");
	NormalMap nor(fi);
	FloatImage depth = nor.construct_depth_map();
	depth.write_to("depth.png");
	HeightMap height(depth);
	height.write_to("result.obj");
	FloatImage recovered = height.generate_normal_map();
	recovered.write_to("recovered.png");

	return 0;
}
