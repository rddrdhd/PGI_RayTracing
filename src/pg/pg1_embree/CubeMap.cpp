#include "stdafx.h"
#include "CubeMap.h"

CubeMap::CubeMap()
{
}

CubeMap::CubeMap(const std::string front_filename, 
	const std::string left_filename, 
	const std::string right_filename,
	const std::string back_filename,
	const std::string up_filename,
	const std::string down_filename)
{
	front_ = Texture(front_filename.c_str());
	left_ = Texture(left_filename.c_str());
	right_ = Texture(right_filename.c_str());
	back_ = Texture(back_filename.c_str());
	up_ = Texture(up_filename.c_str());
	down_ = Texture(down_filename.c_str());
}

Color4f CubeMap::get_texel(const float u, const float v){
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


	
	return Color4f{0,0,0, 1 };
}
