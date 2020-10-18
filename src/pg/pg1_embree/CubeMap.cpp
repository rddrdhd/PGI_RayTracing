#include "stdafx.h"
#include "CubeMap.h"

constexpr auto POS_Z = 0;
constexpr auto NEG_X = 1;
constexpr auto POS_X = 2;
constexpr auto NEG_Z = 3;
constexpr auto POS_Y = 4;
constexpr auto NEG_Y = 5;

CubeMap::CubeMap()
{
	//Texture maps_[6] = { Texture(),Texture(),Texture(),Texture(),Texture(),Texture() };
}

CubeMap::CubeMap(const std::string filename_pos_z, 
	const std::string filename_neg_x, 
	const std::string filename_pos_x,
	const std::string filename_neg_z,
	const std::string filename_pos_y,
	const std::string filename_neg_y)
{
	front_ = Texture(filename_pos_z.c_str());
	left_ = Texture(filename_neg_x.c_str());
	right_ = Texture(filename_pos_x.c_str());
	back_ = Texture(filename_neg_z.c_str());
	up_ = Texture(filename_pos_y.c_str());
	down_ = Texture(filename_neg_y.c_str());
	//maps_ = {front_, left_, right_, back_, up_, down_};
}

Color4f CubeMap::get_texel(Vector3 direction){
	char longest = direction.LargestComponent(true);
	int map = -1;
	float u{}, v{};

	if (longest == 0) { // Z
		map = (direction.z > 0) ? POS_Z : NEG_Z;
		const float tmp = 1.0f / abs(direction.z);
		u = (direction.x * tmp + 1) * 0.5f;
		v = (direction.y * tmp + 1) * 0.5f;
	}
	else if (longest == 1){ // Y
		map = (direction.y > 0) ? POS_Y : NEG_Y;
		const float tmp = 1.0f / abs(direction.y);
		u = (direction.x * tmp + 1) * 0.5f;
		v = (direction.z * tmp + 1) * 0.5f;
	}
	else if (longest == 2){ // X
		map = (direction.x > 0) ? POS_X : NEG_X;
		const float tmp = 1.0f / abs(direction.x);
		u = (direction.y * tmp + 1) * 0.5f;
		v = (direction.z * tmp + 1) * 0.5f;
	}

	//assert( ( u >= 0.0f && u <= 1.0f ) && ( v >= 0.0f && v <= 1.0f ) );	
/*
	const int x = max(0, min(width_ - 1, int(u * width_)));
	const int y = max(0, min(height_ - 1, int(v * height_)));

	const int offset = y * scan_width_ + x * pixel_size_;
	const float b = data_[offset] / 255.0f;
	const float g = data_[offset + 1] / 255.0f;
	const float r = data_[offset + 2] / 255.0f;
	*/
	//return Color4f{ r, g, b, 1 };

	Color3f texel = maps_[map].get_texel(u, v);
	Color4f color = Color4f{ texel.r, texel.g, texel.b, 1 };
	return color;
	
	//return Color4f{0,0,0, 1 };
}
