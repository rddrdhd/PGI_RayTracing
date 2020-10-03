#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"
#include "LightSource.h"

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
	Vector3 getWhittedIlumination();
	bool isIlluminated(LightSource light, RTCRayHit ray_hit);

	int Ui();

private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;
	std::vector<LightSource> lights_;

	RTCDevice device_;
	RTCScene scene_;
	Camera camera_;
};
