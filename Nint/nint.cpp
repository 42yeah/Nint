#include "nint.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <fstream>

//
// We are just going to go ahead and throw errors because this is just a code demo.
//
FloatImage::FloatImage(const std::string& path) : path(path)
{
	int ch;
	unsigned char *raw = stbi_load(path.c_str(), &width, &height, &ch, 0);
	data = std::make_unique<float[]>(width * height * 3);
	if (ch != 4 && ch != 3)
	{
		throw "Wrong number of channels, expected 3 or 4";
	}
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int off_flimage = (y * width + x) * 3;
			int off = (y * width + x) * ch;
			data[off_flimage + 0] = ((float)(raw[off + 0])) / 255.0f;
			data[off_flimage + 1] = ((float)(raw[off + 1])) / 255.0f;
			data[off_flimage + 2] = ((float)(raw[off + 2])) / 255.0f;
		}
	}
}

FloatImage::FloatImage(int width, int height) : width(width), height(height)
{
	data = std::make_unique<float[]>(width * height * 3);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int off_flimage = (y * width + x) * 3;
			data[off_flimage + 0] = 0.0f;
			data[off_flimage + 1] = 0.0f;
			data[off_flimage + 2] = 0.0f;
		}
	}
}

unsigned char to_unsigned_char(float f)
{
	if (f < 0.0f) { f = 0.0f; }
	else if (f > 1.0f) { f = 1.0f; }
	return (unsigned char) (f * 255.0f);
}

void FloatImage::write_to(const std::string& path)
{
	unsigned char* raw = new unsigned char[width * height * 3];
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int off = (y * width + x) * 3;
			raw[off + 0] = to_unsigned_char(data[off + 0]);
			raw[off + 1] = to_unsigned_char(data[off + 1]);
			raw[off + 2] = to_unsigned_char(data[off + 2]);
		}
	}
	if (!stbi_write_png(path.c_str(), width, height, 3, raw, 0))
	{
		throw "Cannot write image, for whatever reason.";
	}
}

int FloatImage::offset(int x, int y, OffsetBehavior behavior)
{
	if (x < 0)
	{
		if (behavior == THROW) { throw "X component out of bound"; }
		else if (behavior == CLAMP_TO_EDGE) { x = 0; }
		else if (behavior == REPEAT) { x = width - 1; }
	}
	else if (x >= width)
	{
		if (behavior == THROW) { throw "X component out of bound"; }
		else if (behavior == CLAMP_TO_EDGE) { x = width - 1; }
		else if (behavior == REPEAT) { x = 0; }
	}
	if (y < 0)
	{
		if (behavior == THROW) { throw "Y component out of bound"; }
		else if (behavior == CLAMP_TO_EDGE) { y = 0; }
		else if (behavior == REPEAT) { y = height - 1; }
	}
	else if (y >= height)
	{
		if (behavior == THROW) { throw "Y omponent out of bound"; }
		else if (behavior == CLAMP_TO_EDGE) { y = height - 1; }
		else if (behavior == REPEAT) { y = 0; }
	}

	return (y * width + x) * 3;
}

NormalMap::NormalMap(FloatImage& fi) : fi(fi)
{
}

FloatImage NormalMap::construct_depth_map() const
{
	//
	// 1. Assume the bottom left corner to be constant depth (0.5).
	// 2. Perform normal integration for each pixel.
	//
	FloatImage ret(fi.width, fi.height);
	for (int y = 0; y < ret.height; y++)
	{
		for (int x = 0; x < ret.width; x++)
		{
			float x_direction_sum = 0.0f;
			float y_direction_sum = 0.0f;
			int valid_xs = 0, valid_ys = 0;
			bool debug = ((x >= 32 && x <= 32) && (y >= 63 && y <= 63));
			for (int dx = 0; dx <= x; dx++)
			{
				float nx = fi.data[fi.offset(dx, y, THROW) + 0] * 2.0f - 1.0f;
				float ny = fi.data[fi.offset(dx, y, THROW) + 1] * 2.0f - 1.0f;
				float nz = fi.data[fi.offset(dx, y, THROW) + 2] * 2.0f - 1.0f;
				if (debug && (dx == 32 && y == 60))
				{
					std::cout << x << "," << y << ": " << nx << ", " << ny << ", " << nz << std::endl;
					std::cout << "Normalized? " << (nx * nx + ny * ny + nz * nz) << std::endl;
					std::cout << "X direction contribution: " << (-nx / nz) / (x + 1) << std::endl;
				}
				if (nz >= 0.001)
				{
					valid_xs++;
					x_direction_sum += (nx / nz);
				}
			}
			if (valid_xs > 0)
			{
				x_direction_sum /= ret.width;
			}
			
			for (int dy = 0; dy <= y; dy++)
			{
				float nx = fi.data[fi.offset(x, dy, THROW) + 0] * 2.0f - 1.0f;
				float ny = fi.data[fi.offset(x, dy, THROW) + 1] * 2.0f - 1.0f;
				float nz = fi.data[fi.offset(x, dy, THROW) + 2] * 2.0f - 1.0f;
				if (debug && (ny > 0.01f || nz > 0.01f))
				{
					// std::cout << x << "," << y << ": " << nx << ", " << ny << ", " << nz << std::endl;
					// std::cout << "Normalized? " << (nx * nx + ny * ny + nz * nz) << std::endl;
					// std::cout << "Y direction contribution: " << (-ny / nz) / (y + 1) << std::endl;
				}
				if (nz >= 0.001)
				{
					valid_ys++;
					y_direction_sum += -(ny / nz);
				}
			}
			if (valid_ys > 0)
			{
				y_direction_sum /= ret.height;
			}
			if (debug)
			{
				std::cout << "Final X and Y contribs of " << x << ", " << y << ": " << x_direction_sum << ", " << y_direction_sum << std::endl;
				std::cout << "Depth: " << (0.5f + x_direction_sum + y_direction_sum) << std::endl;
			}
			
			float d = 0.5f + x_direction_sum + y_direction_sum;
			ret.data[ret.offset(x, y, THROW) + 0] = d;
			ret.data[ret.offset(x, y, THROW) + 1] = d;
			ret.data[ret.offset(x, y, THROW) + 2] = d;
		}
	}

	return ret;
}

HeightMap::HeightMap(FloatImage& fi) : fi(fi)
{
}

//
// Start by the bottom left, write two triangles every time
//
void HeightMap::write_to(const std::string& path)
{
	std::ofstream writer(path);
	writer << "o HeightMap" << std::endl;
	for (int y = 0; y < fi.height; y++)
	{
		for (int x = 0; x < fi.width; x++)
		{
			float x_nor = (float) x / fi.width;
			float y_nor = (float) y / fi.height;
			//
			// Invert d to indicate it's closer to camera
			//
			float d = (1.0f - fi.data[fi.offset(x, y, THROW)]);
			writer << "v " << x_nor << " " << y_nor << " " << d << std::endl;
		}
	}
	for (int y = 1; y < fi.height - 1; y++)
	{
		for (int x = 1; x < fi.width - 1; x++)
		{
			int this_one = (y - 1) * fi.width + (x - 1) + 1;
			int next_one_hor = this_one + 1;
			int next_one_ver = y * fi.width + x;
			int next_one_diag = next_one_ver + 1;
			writer << "f " << this_one << " " << next_one_hor << " " << next_one_diag << std::endl;
			writer << "f " << next_one_diag << " " << next_one_ver << " " << this_one << std::endl;
		}
	}
	writer.close();
}

FloatImage HeightMap::generate_normal_map() const
{
	FloatImage ret(fi.width, fi.height);
	for (int y = 0; y < fi.height; y++)
	{
		for (int x = 0; x < fi.width; x++)
		{
			float x_nor = (float)x / fi.width;
			float y_nor = (float)y / fi.height;
			float z = fi.data[fi.offset(x, y, THROW)];

			float zdu = fi.data[fi.offset(x - 1, y, CLAMP_TO_EDGE)];
			float zdv = fi.data[fi.offset(x, y - 1, CLAMP_TO_EDGE)];

			float du[3] =
			{
				-1.0f / fi.width,
				0.0f,
				z - zdu
			};
			float dv[3] =
			{
				0.0f,
				-1.0f / fi.height,
				z - zdv
			};
			float dul = sqrtf(powf(1.0f / fi.width, 2.0f) + powf(z - zdu, 2.0f));
			float dvl = sqrtf(powf(1.0f / fi.height, 2.0f) + powf(z - zdv, 2.0f));
			float du_nor[3] =
			{
				du[0] / dul, du[1] / dul, du[2] / dul
			};
			float dv_nor[3] =
			{
				dv[0] / dvl, dv[1] / dvl, dv[2] / dvl
			};
			float nor[3] =
			{
				du_nor[1] * dv_nor[2] - du_nor[2] * dv_nor[1],
				du_nor[0] * dv_nor[2] - du_nor[2] * dv_nor[0],
				du_nor[0] * dv_nor[1] - du_nor[1] * dv_nor[0],
			};
			ret.data[ret.offset(x, y, THROW) + 0] = nor[0] * 0.5f + 0.5f;
			ret.data[ret.offset(x, y, THROW) + 1] = nor[1] * 0.5f + 0.5f;
			ret.data[ret.offset(x, y, THROW) + 2] = nor[2] * 0.5f + 0.5f;
		}
	}
	return ret;
}
