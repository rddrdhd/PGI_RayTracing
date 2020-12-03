#include "stdafx.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"
#include "material.h"
#include <iostream>
using namespace std;
// if tx1 < tx0, pak je pot?ebuji swapnout - Ray vs AABB intersect
// pro tx, ty, tz. Je to kvuli tomu, abychom meli konzistentn sematninku - 0 je bliz, 1 je dale
Raytracer::Raytracer( const int width, const int height,
	const float fov_y, const Vector3 view_from, const Vector3 view_at,
	const char * config ) : SimpleGuiDX11( width, height )
{
	InitDeviceAndScene( config );

	camera_ = PinHoleCamera( width, height, fov_y, view_from, view_at );
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


void Raytracer::LoadScene(const std::string object_file_name, const std::string background_file_name)
{
	const int no_surfaces = LoadOBJ(object_file_name.c_str(), surfaces_, materials_);

	// SET svetla
	/*
	Vector3 red(0.8, 0.2, 0.2);
	Vector3 green(0.2, 0.8, 0.2);
	Vector3 blue(0.2, 0.2, 0.8);
	LightSource red_light(Vector3(-10,0,0), red, red, red);
	LightSource green_light(Vector3(0,80,0), green, green, green);
	LightSource blue_light(Vector3(0,0,-50), blue, blue, blue);

	lights_.push_back(red_light);
	lights_.push_back(green_light);
	lights_.push_back(blue_light);
	*/
	Vector3 white(1, 1, 1);
	LightSource white_light(Vector3(10, 0, 0), white, white, white);
	lights_.push_back(white_light);

	this->background_ = SphericalMap(background_file_name);

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

bool Raytracer::isIlluminated(LightSource light, Vector3 hit_position, Vector3 normal)
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

	// Q5 - Hard shadows

	return (light_ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID);
}

RTCRay Raytracer::get_refraction_ray(Vector3 direction, Vector3 normal, float n1, float n2, Vector3 hit_point)
{
	RTCRay refraction_ray;
	direction.Normalize();
	normal.Normalize();

	// Q8 Refraction
	float n1_n2 = (float) (n1 / n2);
	float d_n_ = (float) (direction.DotProduct(normal));
	Vector3 scaled_direction = Vector3(direction.x * n1_n2, direction.y * n1_n2, direction.z * n1_n2);
	Vector3 refraction_direction = ( (scaled_direction)  - (n1_n2 * d_n_ + sqrt( 1 - ( (n1_n2*n1_n2)  * (1 -( d_n_* d_n_))) ) ) * normal);
	
	refraction_ray.org_x = hit_point.x;
	refraction_ray.org_y = hit_point.y;
	refraction_ray.org_z = hit_point.z;

	refraction_ray.dir_x = (refraction_direction.x);
	refraction_ray.dir_y = (refraction_direction.y);
	refraction_ray.dir_z = (refraction_direction.z);

	refraction_ray.tnear = 0.01f;
	refraction_ray.tfar = FLT_MAX;
	refraction_ray.time = n2;

	refraction_ray.mask = 0; // can be used to mask out some geometries for some rays
	refraction_ray.id = 0; // identify a ray inside a callback function
	refraction_ray.flags = 0; // reserved

	return refraction_ray;
}

RTCRay Raytracer::get_reflection_ray(Vector3 direction, Vector3 normal, Vector3 hit_point, float ior)
{
	RTCRay reflection_ray;
	direction.Normalize();
	normal.Normalize();

	// Q7 Reflection
	Vector3 reflection_direction = direction - 2 * (direction.DotProduct(normal) * normal);
	
	reflection_ray.org_x = hit_point.x;
	reflection_ray.org_y = hit_point.y;
	reflection_ray.org_z = hit_point.z;

	reflection_ray.dir_x = reflection_direction.x;
	reflection_ray.dir_y = reflection_direction.y;
	reflection_ray.dir_z = reflection_direction.z;

	reflection_ray.tnear = 0.01f;
	reflection_ray.tfar = (float)FLT_MAX;
	reflection_ray.time = ior;

	reflection_ray.mask = 0; // can be used to mask out some geometries for some rays
	reflection_ray.id = 0; // identify a ray inside a callback function
	reflection_ray.flags = 0; // reserved

	return reflection_ray;
}

Color4f Raytracer::trace(RTCRay ray, int level ) {

	// TODO Q10 Gamma correction http://mrl.cs.vsb.cz/people/fabian/pg1/pr02.pdf
	// TODO Q6 Bliliear interpolation http://mrl.cs.vsb.cz/people/fabian/pg1/pr03.pdf
	// TODO Q Super Sampling http://mrl.cs.vsb.cz/people/fabian/pg1/pr04.pdf
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

	// smer paprsku, ktery prave zpracovavam
	Vector3 primary_ray_direction_vector(ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z);

	if (level >=4) { //level>=4 : 3 minuty
		return this->background_.get_texel(
			primary_ray_direction_vector.x,
			primary_ray_direction_vector.y,
			primary_ray_direction_vector.z);
	}

	if (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID) // we hit something
	{
		RTCGeometry geometry = rtcGetGeometry(scene_, ray_hit.hit.geomID);
		Normal3f normal;

		// get interpolated normal
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);
		Material* material = (Material*)(rtcGetGeometryUserData(geometry));
		Vector3 normal_vector(normal.x, normal.y, normal.z);

		// vypocitam misto hitu
		Vector3 hit_point = Vector3(ray_hit.ray.org_x, ray_hit.ray.org_y, ray_hit.ray.org_z) +
			Vector3(ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z) * ray_hit.ray.tfar;

		Color4f final_color;

		float local_r, local_g, local_b;
		float n1, n2;
		if (ray.time == IOR_AIR) {
			n1 = IOR_AIR;
			n2 = material->ior;
		} else {
			n1 = material->ior;
			n2 = IOR_AIR;
		}

		normal_vector.Normalize();

		primary_ray_direction_vector.Normalize();

		if (normal_vector.DotProduct(primary_ray_direction_vector) > 0) {
			// fix orientace normal
			normal_vector = -normal_vector;
		}

		// Q2 normal shader - abychom mohli promitnout normaly jako barvy
		/*float nx = (((normal_vector.x) + 1) / 2);
		float ny = (((normal_vector.y) + 1) / 2);
		float nz = (((normal_vector.z) + 1) / 2);
		Vector3 n(nx, ny, nz);

		return Color4f{ n.x, n.y,n.z, 1.0f };*/

		float R = 0.0f;
		Vector3 v = -primary_ray_direction_vector;

		float cos1, cos2, Rs, Rp;
		cos1 = abs(normal_vector.DotProduct(v));
		float n1_n2_twice = ((n1 / n2) * (n1 / n2));
		float one_minus_cos1_pow2 = (1 - (cos1 * cos1));
		cos2 = sqrt(1 - (n1_n2_twice * one_minus_cos1_pow2));

		float uhel_2 =(float) (acos(cos2) * (180.0 / 3.141592653589793238463)) ;
		Rs = ((n2 * cos2 - n1 * cos1) / (n2 * cos2 + n1 * cos1)) * ((n2 * cos2 - n1 * cos1) / (n2 * cos2 + n1 * cos1));
		Rp = ((n2 * cos1 - n1 * cos2) / (n2 * cos1 + n1 * cos2)) * ((n2 * cos1 - n1 * cos2) / (n2 * cos1 + n1 * cos2));
		R = (Rs + Rp) / 2;

		switch (material->type) {
		case 4: //pruhledny a leskly material
			RTCRay reflection_ray;
			RTCRay refraction_ray;

			// reflection
			reflection_ray = get_reflection_ray(primary_ray_direction_vector, normal_vector, hit_point, n1);
			Color4f reflection_color = trace(reflection_ray, level+1);			

			//refraction
			refraction_ray = get_refraction_ray(primary_ray_direction_vector, normal_vector, n1, n2, hit_point);
			Color4f refraction_color = trace(refraction_ray, level+1);
			
			// Q3 Lambert - Attenuation of Refracted Ray
			// to know how long is the ray *inside*
			RTCHit refraction_end;
			refraction_end.geomID = RTC_INVALID_GEOMETRY_ID;
			refraction_end.primID = RTC_INVALID_GEOMETRY_ID;
			refraction_end.Ng_x = 0.0f; // geometry normal
			refraction_end.Ng_y = 0.0f;
			refraction_end.Ng_z = 0.0f;

			RTCRayHit refraction_ray_hit;
			refraction_ray_hit.ray = refraction_ray;
			refraction_ray_hit.hit = refraction_end;
			
			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect1(scene_, &context, &refraction_ray_hit);

			local_r = exp(-(1 - material->diffuse.x));
			local_g = exp(-(1 - material->diffuse.y));
			local_b = exp(-(1 - material->diffuse.z));

			// if we are *inside* and hit the end of *inside*
			if (refraction_ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID) { 
				local_r = exp(-(1 - material->diffuse.x) * refraction_ray_hit.ray.tfar);
				local_g = exp(-(1 - material->diffuse.y) * refraction_ray_hit.ray.tfar);
				local_b = exp(-(1 - material->diffuse.z) * refraction_ray_hit.ray.tfar);
			}

			final_color = {
				((reflection_color.b * R) + (refraction_color.b * (1 - R))) * local_r,
				((reflection_color.g * R) + (refraction_color.g * (1 - R))) * local_g,
				((reflection_color.r * R) + (refraction_color.r * (1 - R))) * local_b,
				1.0f
			};
			return final_color;
			break;
		default:
			
			float r = 0;
			float g = 0;
			float b = 0;

			for (LightSource light : this->lights_) {

				if (isIlluminated(light, Vector3(hit_point.x, hit_point.y, hit_point.z), Vector3(normal.x, normal.y, normal.z))) {

					//vypocitam vektor z hitu do svetla
					Vector3 l(hit_point.x - light.position_.x, hit_point.y - light.position_.y, hit_point.z - light.position_.z);
					l.Normalize();

					// vypocitam vektor z hitu do oka
					Vector3 v(ray.org_x - ray.dir_x, ray.org_y - ray.dir_y, ray.org_z - ray.dir_z);
					v.Normalize();

					Vector3 l_r = 2 * (normal_vector.DotProduct(l)) * normal_vector - l;
					l_r.Normalize();
					double gamma = material->shininess;

					// vypocitam jednotlive barvy i s utlumem
					double i_d = light.diffuse_.x;
					double m_d = material->diffuse.x;
					double i_s = light.spectular_.x;
					double m_s = material->specular.x;

					// Q4 Whitted illumination model
					r += (i_d * m_d * (normal_vector.DotProduct(l)) + i_s * m_s * (pow(v.DotProduct(l_r), gamma)));

					i_d = light.diffuse_.y;
					m_d = material->diffuse.y;
					i_s = light.spectular_.y;
					m_s = material->specular.y;
					g += (i_d * m_d * (normal_vector.DotProduct(l)) + i_s * m_s * (pow(v.DotProduct(l_r), gamma)));

					i_d = light.diffuse_.z;
					m_d = material->diffuse.z;
					i_s = light.spectular_.z;
					m_s = material->specular.z;
					b += (i_d * m_d * (normal_vector.DotProduct(l)) + i_s * m_s * (pow(v.DotProduct(l_r), gamma)));

				}
			}

			Vector3 ambient = material->ambient * 0.5;
			return Color4f{ r + ambient.x, g + ambient.y, b + ambient.z, 1.0f };
		}

	}
	// pokud se nic nestalo, vratim pozadi
	return this->background_.get_texel(
		primary_ray_direction_vector.x,
		primary_ray_direction_vector.y,
		primary_ray_direction_vector.z);;
}

Color4f Raytracer::get_pixel( const int x, const int y, const float t )
{
	// generate primary ray and perform ray cast on the scene
	RTCRay primary_ray = camera_.GenerateRay(x, y);
	primary_ray.time = IOR_AIR;
	return trace(primary_ray, 0);
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


