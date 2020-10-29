#define _USE_MATH_DEFINES
#include "stdafx.h"
#include "SphericalMap.h"
#include "corecrt_math_defines.h"

SphericalMap::SphericalMap()
{
	const std::string file_name = "../../../data/spherical_map_rails.jpg";
	// 1 load spherical texture from file_name and store it in texture_
	texture_ = std::make_unique<Texture>(Texture(file_name.c_str()));
}

SphericalMap::SphericalMap(const std::string& file_name)
{
	// 1 load spherical texture from file_name and store it in texture_
	texture_ = std::make_unique<Texture>(Texture(file_name.c_str()));
}

Color4f SphericalMap::get_texel(const float x, const float z, const float y) const
{
	Vector3 vec{ x, y, z };
	float u = 0.5 + atan2(vec.x, vec.z) / (2 * M_PI);
	float v = 0.5 - asin(vec.y) / M_PI;
	Color3f c = this->texture_->get_texel(u, v);

	return Color4f{c.r, c.g, c.b, 1};
}