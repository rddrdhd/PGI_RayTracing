#pragma once
#include "texture.h"
#include "vector3.h"
class CubeMap
{
public:
	CubeMap();
	CubeMap(const std::string front_filename, const std::string left_filename, const std::string right_filename, const std::string back_filename, const std::string up_filename, const std::string down_filename);
	Color4f get_texel(const float u, const float v);

private:
	Texture front_;
	Texture left_;
	Texture right_;
	Texture back_;
	Texture down_;
	Texture up_;
};

