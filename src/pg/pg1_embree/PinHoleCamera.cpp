#include "stdafx.h"
#include "PinHoleCamera.h"

// Q1 Pinhole camera
PinHoleCamera::PinHoleCamera( const int width, const int height, const float fov_y,
	const Vector3 view_from, const Vector3 view_at )
{
	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	//compute focal lenght based on the vertical field of view and the camera resolution
	f_y_ = height_ / (2 * _CMATH_::tan(fov_y_ / 2)); // focal length = vyska / 2tan(uhel/2)

	// vektory pro transformaci prostoru z kameroveho do svetoveho (pravotocivy!)
	Vector3 z_c = view_from_ - view_at_; // smer pohledu kamery na prostor
	Vector3 x_c = up_.CrossProduct(z_c); // kolmice mezi pohledem a svislou osou
	Vector3 y_c = z_c.CrossProduct(x_c); // 𝒂×𝒃= 𝑎𝑦𝑏𝑧−𝑎𝑧𝑏𝑦, 𝑎𝑧𝑏𝑥−𝑎𝑥𝑏𝑧, 𝑎𝑥𝑏𝑦−𝑎𝑦𝑏𝑥

	// Matice pro prevod mezi soustavami musi byt z jednotkovych vektoru !
	z_c.Normalize();
	x_c.Normalize();
	y_c.Normalize();

	M_c_w_ = Matrix3x3( x_c, y_c, z_c );
}

RTCRay PinHoleCamera::GenerateRay( const float x_i, const float y_i ) const
{
	RTCRay ray = RTCRay();

	ray.org_x = view_from_.x;
	ray.org_y = view_from_.y;
	ray.org_z = view_from_.z;

	// smerovy vektor kamery
	Vector3 ray_direction(
		x_i - (width_ / 2),
		(height_ / 2) - y_i,
		-f_y_ // focal length
	);

	ray_direction.Normalize();

	Vector3 direction = M_c_w_* ray_direction;

	ray.dir_x = direction.x;
	ray.dir_y = direction.y;
	ray.dir_z = direction.z;

	ray.tnear = 0.001f;
	ray.tfar = FLT_MAX;
	ray.time = 0.0f;

	ray.mask = 0; // can be used to mask out some geometries for some rays
	ray.id = 0; // identify a ray inside a callback function
	ray.flags = 0; // reserved
	
	return ray;
}
RTCRay PinHoleCamera::GenerateRay( const float x_i, const float y_i, const float focal_length, const float aperture ) const
{
	Vector3 camera_ray_direction(x_i - width_ / 2, height_ / 2 - y_i, -f_y_);
	camera_ray_direction.Normalize();
	Vector3 direction_ws = M_c_w_ * camera_ray_direction;// to world coord system
	direction_ws.Normalize();

	// get the WS coordiantes of point P which lies on the primary ray at the distance fr (radius)
	//Vector3 focal_point = Vector3{ view_from_.x, view_from_.y , view_from_.z } + Vector3{ direction }*focal_length;
	Vector3 focal_point = Vector3{view_from_.x+direction_ws.x*focal_length, view_from_.y + direction_ws.y * focal_length , view_from_.z + direction_ws.z * focal_length } ;
	
	// in random manner shift the origin of the primary ray using the aperture size
	std::mt19937 generator(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	std::uniform_real_distribution<float> uni_dist(-aperture / 2.0f, aperture / 2.0f); 
	float rand1 = uni_dist(generator);
	float rand2 = uni_dist(generator);
	Vector3 shift = M_c_w_ * Vector3{ rand1, rand2, 0 };

	RTCRay ray = RTCRay();
	ray.org_x = view_from_.x +shift.x;
	ray.org_y = view_from_.y +shift.y;
	ray.org_z = view_from_.z +shift.z;

	// set the direction vector of the primary so that it goes through the focal point P
	Vector3 new_direction{ focal_point.x-ray.org_x,  focal_point.y - ray.org_y , focal_point.z - ray.org_z };
	new_direction.Normalize();

	ray.dir_x = new_direction.x;
	ray.dir_y = new_direction.y;
	ray.dir_z = new_direction.z;//aaaaaaaaaaaaaaaaaaaaa

	ray.tnear = 0.01f;
	ray.tfar = FLT_MAX;
	ray.time = 0.0f;

	ray.mask = 0; // can be used to mask out some geometries for some rays
	ray.id = 0; // identify a ray inside a callback function
	ray.flags = 0; // reserved

	return ray;
}

Vector3 PinHoleCamera::get_origin()
{
	return this->view_from_;
}

Vector3 PinHoleCamera::get_direction()
{
	return this->view_at_;
}
