#include "stdafx.h"
#include "LightSource.h"
LightSource::LightSource(Vector3 position,Vector3 ambient, Vector3 diffuse, Vector3 spectular )
{
	position_ = position;
	ambient_ = ambient;
	diffuse_ = diffuse;
	spectular_ = spectular;
}

RTCRay LightSource::GenerateRay(const float x, const float y, const float z) const
{
	RTCRay ray = RTCRay();
	ray.org_x = this->position_.x;
	ray.org_y = this->position_.y;
	ray.org_z = this->position_.z;

	ray.dir_x = x;
	ray.dir_y = y; 
	ray.dir_z = z;

	Vector3 dir(x, y, z);
	
	ray.tnear = FLT_MIN;

	// vzdalenost k hitu
	ray.tfar = Vector3(position_-dir).L2Norm();

	ray.time = 0.0f;

	return ray;
}