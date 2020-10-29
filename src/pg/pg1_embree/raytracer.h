#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"
#include "LightSource.h"
#include "CubeMap.h"
#include "SphericalMap.h"

/*! \class Raytracer
\brief General ray tracer class.

\author Tomáš Fabián
\version 0.1
\date 2018
*/
class Raytracer : public SimpleGuiDX11
{
public:
	Raytracer( const int width, const int height, 
		const float fov_y, const Vector3 view_from, const Vector3 view_at,
		const char * config = "threads=0,verbose=3" );
	~Raytracer();

	int InitDeviceAndScene( const char * config );

	int ReleaseDeviceAndScene();

	void LoadScene( const std::string file_name );

	Color4f trace( RTCRay ray, int level );
	Color4f get_pixel( const int x, const int y, const float t = 0.0f ) override;
	bool isIlluminated(LightSource light, Vector3 hit_position, Vector3 normal);

	int Ui();


	RTCRay get_refraction_ray(Vector3 direction, Vector3 normal, float iorFrom, float iorTo, Vector3 hit_point);
	RTCRay get_reflection_ray(Vector3 direction, Vector3 normal, Vector3 hit_point, float ior);


private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;
	std::vector<LightSource> lights_;

	//CubeMap background_;
	SphericalMap background_;
	RTCDevice device_;
	RTCScene scene_;
	Camera camera_;
};
