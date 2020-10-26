#pragma once
#include "texture.h"
#include "vector3.h"
class SphericalMap
{
public:
	SphericalMap();
	SphericalMap(const std::string& file_name);
	Color4f get_texel(const float x, const float y, const float z) const;
private:

	std::unique_ptr<Texture> texture_;
};

