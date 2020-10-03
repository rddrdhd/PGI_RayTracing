#pragma once
#include "vector3.h"
class LightSource
{
public:
	LightSource(Vector3 position, Vector3 ambient, Vector3 diffuse, Vector3 spectular);

	/* generate primary ray, top-left pixel image coordinates (xi, yi) are in the range <0, 1) x <0, 1) */
	RTCRay GenerateRay(const float xi, const float yi, const float zi) const;

	bool Illuminates(RTCRayHit ray_hit);
private:
	Vector3 position_;

	Vector3 ambient_;
	Vector3 diffuse_;
	Vector3 spectular_;
};

