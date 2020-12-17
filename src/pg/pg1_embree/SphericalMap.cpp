#define _USE_MATH_DEFINES
#include "stdafx.h"
#include "SphericalMap.h"
#include "corecrt_math_defines.h"

SphericalMap::SphericalMap()
{
	const std::string file_name = "../../../data/spherical_map_windows.jpg";
	texture_ = std::make_unique<Texture>(Texture(file_name.c_str()));
}

SphericalMap::SphericalMap(const std::string& file_name) 
{
	texture_ = std::make_unique<Texture>(Texture(file_name.c_str()));
}

Color4f SphericalMap::get_texel(const float x, const float y, const float z) const
{
	// Q9 Environmental map
	Vector3 vec{ x, z, y };
	float u = 0.5 + atan2(vec.x, vec.z) / (2 * M_PI);
	float v = 0.5 - asin(vec.y) / M_PI;
	Color3f c = this->texture_->get_texel(u, v);


	//return Color4f{1,1,1,1};
	return Color4f{c.b, c.g, c.r, 1};
}