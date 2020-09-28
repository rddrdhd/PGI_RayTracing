#include "stdafx.h"
#include "camera.h"

Camera::Camera( const int width, const int height, const float fov_y,
	const Vector3 view_from, const Vector3 view_at )
{
	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	// TODO compute focal lenght based on the vertical field of view and the camera resolution
	
	f_y_ = height_ / (2 * _CMATH_::tan(fov_y_ / 2)); // focal length = vyska / 2tan(uhel/2)

	// TODO build M_c_w_ matrix	

	// vektory pro transformaci prostoru z kameroveho do svetoveho (pravotocivy!)


	Vector3 z_c = view_from_ - view_at_; // pohled kamery na prostor
	Vector3 x_c = up_.CrossProduct(z_c); // kolmice mezi pohledem a svislou osou
	Vector3 y_c = z_c.CrossProduct(x_c); // 𝒂×𝒃= 𝑎𝑦𝑏𝑧−𝑎𝑧𝑏𝑦, 𝑎𝑧𝑏𝑥−𝑎𝑥𝑏𝑧, 𝑎𝑥𝑏𝑦−𝑎𝑦𝑏𝑥

	// Matice musi byt z jednotkovych vektoru !
	z_c.Normalize();
	x_c.Normalize();
	y_c.Normalize();

	M_c_w_ = Matrix3x3( x_c, y_c, z_c );
}

RTCRay Camera::GenerateRay( const float x_i, const float y_i ) const
{
	RTCRay ray = RTCRay();

	// TODO fill in ray structure and compute ray direction
	ray.org_x = view_from_.x;
	ray.org_y = view_from_.y;
	ray.org_z = view_from_.z;

	Vector3 d_c(
		x_i - (width_ / 2),
		(height_ / 2) - y_i,
		-f_y_ // focal length
	);

	d_c.Normalize();

	Vector3 direction = M_c_w_* d_c;

	ray.dir_x = direction.x;
	ray.dir_y = direction.y;
	ray.dir_z = direction.z;

	ray.tnear = FLT_MIN;
	ray.tfar = FLT_MAX;
	ray.time = 0.0f;

	ray.mask = 0; // can be used to mask out some geometries for some rays
	ray.id = 0; // identify a ray inside a callback function
	ray.flags = 0; // reserved
	
	return ray;
}
