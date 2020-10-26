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
Color4f SphericalMap::get_texel(const float x, const float y, const float z) const
{

	Vector3 vec{ -y,z, -x}; //vsekladne = vzhuru nohama, -y = zrcadlove, -z = zrcadlove a divne, -x = vzhurunohama
	vec.Normalize(); //vsezaporne = vzhuru nohama, +x = vzhuru nohama, +z dobre ale divne roztahle, +y to same?
	float phi = atan(vec.y / vec.x) + M_PI / 2;
	float theta = acos(vec.z);
	while (theta < 0) {
		theta += 2 * M_PI;
	}

	float u = (theta / (2 * M_PI));
	float v = (phi / M_PI);
	Color3f color = this->texture_->get_texel(u, v);
	return Color4f{color.r, color.g, color.b, 1};
}