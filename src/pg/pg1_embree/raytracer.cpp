#include "stdafx.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"
#include <iostream>
using namespace std;

#define BACKGROUND_COLOR Color4f{ 0.1f, 0.1f, 0.1f, 1.0f };

Raytracer::Raytracer( const int width, const int height,
	const float fov_y, const Vector3 view_from, const Vector3 view_at,
	const char * config ) : SimpleGuiDX11( width, height )
{
	InitDeviceAndScene( config );

	camera_ = Camera( width, height, fov_y, view_from, view_at );
	/*
	background_ = CubeMap(
		"../../../data/PalmTrees/posz.jpg",
		"../../../data/PalmTrees/negx.jpg",
		"../../../data/PalmTrees/posx.jpg",
		"../../../data/PalmTrees/negz.jpg",
		"../../../data/PalmTrees/posy.jpg",
		"../../../data/PalmTrees/negy.jpg");*/
}

Raytracer::~Raytracer()
{
	ReleaseDeviceAndScene();
}

int Raytracer::InitDeviceAndScene( const char * config )
{
	device_ = rtcNewDevice( config );
	error_handler( nullptr, rtcGetDeviceError( device_ ), "Unable to create a new device.\n" );
	rtcSetDeviceErrorFunction( device_, error_handler, nullptr );

	ssize_t triangle_supported = rtcGetDeviceProperty( device_, RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED );

	// create a new scene bound to the specified device
	scene_ = rtcNewScene( device_ );

	return S_OK;
}

int Raytracer::ReleaseDeviceAndScene()
{
	rtcReleaseScene( scene_ );
	rtcReleaseDevice( device_ );

	return S_OK;
}


void Raytracer::LoadScene(const std::string file_name)
{
	//std::string path = "../../../data/PalmTrees/";

	const int no_surfaces = LoadOBJ(file_name.c_str(), surfaces_, materials_);

	// add lights
	Vector3 red(0.8, 0.2, 0.2);
	Vector3 green(0.2, 0.8, 0.2);
	Vector3 blue(0.2, 0.2, 0.8);
	Vector3 white(1, 1, 1);
	LightSource red_light(Vector3(-10,0,0), red, red, red);
	LightSource green_light(Vector3(0,80,0), green, green, green);
	LightSource blue_light(Vector3(0,0,-50), blue, blue, blue);
	LightSource white_light(Vector3(10, 0, 0), white, white, white);

	//lights_.push_back(red_light);
	//lights_.push_back(green_light);
	//lights_.push_back(blue_light);
	lights_.push_back(white_light);

	// surfaces loop
	for ( auto surface : surfaces_ )
	{
		RTCGeometry mesh = rtcNewGeometry( device_, RTC_GEOMETRY_TYPE_TRIANGLE );

		Vertex3f * vertices = ( Vertex3f * )rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
			sizeof( Vertex3f ), 3 * surface->no_triangles() );

		Triangle3ui * triangles = ( Triangle3ui * )rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
			sizeof( Triangle3ui ), surface->no_triangles() );

		rtcSetGeometryUserData( mesh, ( void* )( surface->get_material() ) );

		rtcSetGeometryVertexAttributeCount( mesh, 2 );

		Normal3f * normals = ( Normal3f * )rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3,
			sizeof( Normal3f ), 3 * surface->no_triangles() );

		Coord2f * tex_coords = ( Coord2f * )rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT2,
			sizeof( Coord2f ), 3 * surface->no_triangles() );		

		// triangles loop
		for ( int i = 0, k = 0; i < surface->no_triangles(); ++i )
		{
			Triangle & triangle = surface->get_triangle( i );

			// vertices loop
			for ( int j = 0; j < 3; ++j, ++k )
			{
				const Vertex & vertex = triangle.vertex( j );

				vertices[k].x = vertex.position.x;
				vertices[k].y = vertex.position.y;
				vertices[k].z = vertex.position.z;

				normals[k].x = vertex.normal.x;
				normals[k].y = vertex.normal.y;
				normals[k].z = vertex.normal.z;

				tex_coords[k].u = vertex.texture_coords[0].u;
				tex_coords[k].v = vertex.texture_coords[0].v;
			} // end of vertices loop

			triangles[i].v0 = k - 3;
			triangles[i].v1 = k - 2;
			triangles[i].v2 = k - 1;
		} // end of triangles loop

		rtcCommitGeometry( mesh );
		unsigned int geom_id = rtcAttachGeometry( scene_, mesh );
		rtcReleaseGeometry( mesh );
	} // end of surfaces loop

	rtcCommitScene( scene_ );
}

bool Raytracer::isIlluminated(LightSource light, Vector3 hit_position)
{
	RTCHit light_hit;
	light_hit.geomID = RTC_INVALID_GEOMETRY_ID;
	light_hit.primID = RTC_INVALID_GEOMETRY_ID;
	light_hit.Ng_x = 0.0f; // geometry normal
	light_hit.Ng_y = 0.0f;
	light_hit.Ng_z = 0.0f;

	RTCRay light_ray = light.GenerateRay(hit_position.x, hit_position.y, hit_position.z);

	RTCRayHit light_ray_hit;
	light_ray_hit.ray = light_ray;
	light_ray_hit.hit = light_hit;

	// intersect ray with the scene
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene_, &context, &light_ray_hit);

	return (light_ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID);
}

Color4f Raytracer::trace(RTCRay ray, int level) {
	// omezeni zanoreni
	if(level>=10){
		// TODO!
		return BACKGROUND_COLOR;
	}

	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.primID = RTC_INVALID_GEOMETRY_ID;
	hit.Ng_x = 0.0f; 
	hit.Ng_y = 0.0f;
	hit.Ng_z = 0.0f;

	// merge ray and hit structures
	RTCRayHit ray_hit;
	ray_hit.ray = ray;
	ray_hit.hit = hit;

	// intersect ray with the scene
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene_, &context, &ray_hit);

	Vector3 direction_vector(ray.dir_x, ray.dir_y, ray.dir_z);

	if (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID) // we hit something
	{
		RTCGeometry geometry = rtcGetGeometry(scene_, ray_hit.hit.geomID);
		Normal3f normal;

		// get interpolated normal
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);
		Material* material = (Material*)(rtcGetGeometryUserData(geometry));
		Vector3 normal_vector(normal.x, normal.y, normal.z);

		normal_vector.Normalize();
		direction_vector.Normalize();

		if (normal_vector.DotProduct(direction_vector) > 0) {
			// fix orientace normal
			normal_vector = -normal_vector; 
		} 
		
		/*
		// abchom mohli promitnout normaly jako barvy
		float nx = (((normal_vector.x) + 1) / 2);
		float ny = (((normal_vector.y) + 1) / 2);
		float nz = (((normal_vector.z) + 1) / 2);
		Vector3 n(nx, ny, nz);*/

		float r = 0;
		float g = 0;
		float b = 0;

		for (LightSource light : this->lights_) {
			// vypocitam misto hitu
			Vector3 hit_point = Vector3(ray_hit.ray.org_x, ray_hit.ray.org_y, ray_hit.ray.org_z) +
				Vector3(ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z) * ray_hit.ray.tfar;
			
			if (isIlluminated(light, Vector3(hit_point.x, hit_point.y, hit_point.z))) {
				// TODO!
				//if (material->diffuse.z < 0.01) trace(ray, 2);
				normal_vector.Normalize();

				//vypocitam vektor z hitu do svetla
				Vector3 l(hit_point.x - light.position_.x, hit_point.y - light.position_.y, hit_point.z - light.position_.z);
				l.Normalize();
				// vypocitam vektor z hitu do oka
				Vector3 v(ray.org_x - ray.dir_x, ray.org_y - ray.dir_y, ray.org_z - ray.dir_z);
				v.Normalize();
				Vector3 l_r = 2 * (normal_vector.DotProduct(l)) * normal_vector - l;
				l_r.Normalize();
				double gamma = material->shininess;

				double i_d = light.diffuse_.x;
				double m_d = material->diffuse.x;
				double i_s = light.spectular_.x;
				double m_s = material->specular.x;
				r += (i_d*m_d * (normal_vector.DotProduct(l)) + i_s*m_s*(pow(v.DotProduct(l_r),gamma)));

				i_d = light.diffuse_.y;
				m_d = material->diffuse.y;
				i_s = light.spectular_.y;
				m_s = material->specular.y;
				g += (i_d*m_d * (normal_vector.DotProduct(l)) + i_s*m_s*(pow(v.DotProduct(l_r),gamma)));

				i_d = light.diffuse_.z;
				m_d = material->diffuse.z;
				i_s = light.spectular_.z;
				m_s = material->specular.z;
				b += (i_d*m_d * (normal_vector.DotProduct(l)) + i_s*m_s*(pow(v.DotProduct(l_r),gamma)));
					
			}
		}

		Vector3 ambient = material->ambient * 0.5;
		return Color4f{ r+ambient.x, g+ambient.y, b+ambient.z, 1.0f };
	}
	//Color4f background = background_.get_texel(direction_vector);
	return BACKGROUND_COLOR ;
}

Color4f Raytracer::get_pixel( const int x, const int y, const float t )
{
	// generate primary ray and perform ray cast on the scene
	RTCRay primary_ray = camera_.GenerateRay(x, y);
	return trace(primary_ray, 1);
}

int Raytracer::Ui()
{
	static float f = 0.0f;
	static int counter = 0;

	// Use a Begin/End pair to created a named window
	ImGui::Begin( "Ray Tracer Params" );
	
	ImGui::Text( "Surfaces = %d", surfaces_.size() );
	ImGui::Text( "Materials = %d", materials_.size() );
	ImGui::Separator();
	ImGui::Checkbox( "Vsync", &vsync_ );
	
	//ImGui::Checkbox( "Demo Window", &show_demo_window ); // Edit bools storing our window open/close state
	//ImGui::Checkbox( "Another Window", &show_another_window );

	ImGui::SliderFloat( "float", &f, 0.0f, 1.0f ); // Edit 1 float using a slider from 0.0f to 1.0f 

	//ImGui::ColorEdit3( "clear color", ( float* )&clear_color ); // Edit 3 floats representing a color

	// Buttons return true when clicked (most widgets return true when edited/activated)
	if ( ImGui::Button( "Button" ) )
		counter++;
	ImGui::SameLine();
	ImGui::Text( "counter = %d", counter );

	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
	ImGui::End();

	// 3. Show another simple window.
	/*if ( show_another_window )
	{
	ImGui::Begin( "Another Window", &show_another_window ); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text( "Hello from another window!" );
	if ( ImGui::Button( "Close Me" ) )
	show_another_window = false;
	ImGui::End();
	}*/

	return 0;
}
