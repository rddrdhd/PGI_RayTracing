#include "stdafx.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"
#include "material.h"
#include "utils.h"
#include <iostream>
using namespace std;
// if tx1 < tx0, pak je porebuji swapnout - Ray vs AABB intersect
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
	this->background_ = SphericalMap(background_file_name);
	
	 //Barevna svetla
/*	Vector3 red(0.8, 0.2, 0.2);
	Vector3 green(0.2, 0.8, 0.2);
	Vector3 blue(0.2, 0.2, 0.8);
	LightSource red_light(Vector3(-10,0,0), red, red, red);
	LightSource green_light(Vector3(0,80,0), green, green, green);
	LightSource blue_light(Vector3(0,0,-50), blue, blue, blue);

	lights_.push_back(red_light);
	lights_.push_back(green_light);
	lights_.push_back(blue_light);
	*/
	// Bile svetlo
	Vector3 white(1, 1, 1);
	LightSource white_light(Vector3(-500,-100,500), white, white, white);
	//LightSource white_light(Vector3(300,100,-100), white, white, white);
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

RTCRayHit get_ray_hit(RTCRay ray, RTCScene scene) {
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
	rtcIntersect1(scene, &context, &ray_hit);
	return ray_hit;
};

bool Raytracer::isIlluminated(LightSource light, Vector3 hit_position, Vector3 normal)
{
	RTCRay ray = light.GenerateRay(hit_position.x, hit_position.y, hit_position.z);
	RTCRayHit rh = get_ray_hit(ray, scene_);

	if (rh.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
		RTCGeometry geometry = rtcGetGeometry(scene_, rh.hit.geomID);
		Material* material = (Material*)(rtcGetGeometryUserData(geometry));
		if (material->type == 4) {
			Vector3 org{ rh.ray.org_x, rh.ray.org_y, rh.ray.org_z };
			Vector3 d{ rh.ray.dir_x, rh.ray.dir_y, rh.ray.dir_z };
			Vector3 new_hitPoint = org + d * rh.ray.tfar;
			return isIlluminated(light, new_hitPoint, normal);
		}
		return false;
	}
	// Q5 - Hard shadows
	return true;// (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID);
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
	
	// TODO Q11 Super Sampling, Q12 Depth of field,  Q13 Bounding Volume Hierarchy (construction and traversal), Q14 Surface Area Heuristic  http://mrl.cs.vsb.cz/people/fabian/pg1/pr04.pdf
	
	RTCRayHit ray_hit = get_ray_hit(ray, scene_);

	// smer paprsku, ktery prave zpracovavam
	Vector3 direction_vector(ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z);

	direction_vector.Normalize();
	if (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID) // we hit something
	{
		RTCGeometry geometry = rtcGetGeometry(scene_, ray_hit.hit.geomID);
		Normal3f normal;

		// get interpolated normal
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);
		
		Material* material = (Material*)(rtcGetGeometryUserData(geometry));
	
		Vector3 normal_vector(normal.x, normal.y, normal.z);
		normal_vector.Normalize();

		// vypocitam misto hitu
		Vector3 hit_vector = Vector3(ray_hit.ray.org_x, ray_hit.ray.org_y, ray_hit.ray.org_z) +
			Vector3(ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z) * ray_hit.ray.tfar;

		float n1, n2;// attenuation_red, attenuation_green, attenuation_blue;
		
		if (ray.time == IOR_AIR) {
			n1 = IOR_AIR; // ray is in air
			n2 = material->ior; // ray is hitting material
		} else {
			n1 = material->ior;// ray is in material
			n2 = IOR_AIR; // ray is going out
		}

		if (normal_vector.DotProduct(direction_vector) > 0) {
			// fix normal orientation
			normal_vector = -normal_vector;
		}

		// Q2 normal shader - abychom mohli promitnout normaly jako barvy
		/*float nx = (((normal_vector.x) + 1) / 2);
		float ny = (((normal_vector.y) + 1) / 2);
		float nz = (((normal_vector.z) + 1) / 2);
		Vector3 n(nx, ny, nz);

		return Color4f{ n.x, n.y,n.z, 1.0f };*/


		if (level >= 10) { // 
			return Color4f{ 0,0,0,1 };/*this->background_.get_texel(
				direction_vector.x,
				direction_vector.y,
				direction_vector.z);*/
		}

		Vector3 v = -direction_vector;
		float R, alpha, F0, cos1;

		switch (material->type) {
		case 4: // shader 4 = dielectric = reflection & refraction
			RTCRay reflection_ray, refraction_ray;
			Color4f reflection_color, refraction_color, attenuation;

			// reflection
			reflection_ray = get_reflection_ray(direction_vector, normal_vector, hit_vector, n1);
			reflection_color = trace(reflection_ray, level + 1);


			attenuation.r = exp(-(1 - material->diffuse.x) * ray_hit.ray.tfar);
			attenuation.g = exp(-(1 - material->diffuse.y) * ray_hit.ray.tfar);
			attenuation.b = exp(-(1 - material->diffuse.z) * ray_hit.ray.tfar);

			// refraction
			refraction_ray = get_refraction_ray(direction_vector, normal_vector, n1, n2, hit_vector);
			if (refraction_ray.dir_x == refraction_ray.dir_x) {
				refraction_color = trace(refraction_ray, level + 1);

				// Q3 Lambert - Attenuation of Refracted Ray
				// compute the ratio between reflection and refraction
				cos1 = abs(normal_vector.DotProduct(v));
				alpha = (n1 - n2) / (n1 + n2);
				R = (alpha * alpha + (1 - (alpha * alpha))) * pow((1 - cos1), 5);

				Color4f c = mix_srgb(reflection_color, refraction_color, R);
				return Color4f{ c.r * attenuation.r, c.g * attenuation.g, c.b * attenuation.b, 1 };
			}
			return Color4f{ reflection_color.r*attenuation.r,reflection_color.g * attenuation.g,reflection_color.b * attenuation.b,1 };
			
			break;

		default: // Q4 Phong
			// define final color variables
			float blue = 0; 
			float green = 0;
			float red = 0;
			// define helper variables for formulas
			float m_d_r, m_d_g, m_d_b; // material diffuse
			float i_d_r, i_d_g, i_d_b; // light diffuse
			float i_s_r, i_s_g, i_s_b; // light spectacular
			float m_s_r, m_s_g, m_s_b; // material spectacular
			float gamma;
			// texture
			Texture* diffuseTexture = material->get_texture(Material::kDiffuseMapSlot);
			if (diffuseTexture == nullptr) {
				m_d_r = material->diffuse.z;
				m_d_g = material->diffuse.y;
				m_d_b = material->diffuse.x;
			} else {
				Coord2f tex_coord;
				rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &tex_coord.u, 2);
				Color3f texel = diffuseTexture->get_texel(tex_coord.u, 1.0f - tex_coord.v);
				m_d_r = texel.r;
				m_d_g = texel.g;
				m_d_b = texel.b;
			}

			for (LightSource light : this->lights_) {
				if (isIlluminated(light, hit_vector, normal_vector)) {
					Vector3 light_vector, camera_vector;
					
					//  hit to light
					light_vector = Vector3(light.position_.x- hit_vector.x ,  light.position_.y- hit_vector.y, light.position_.z- hit_vector.z);
					light_vector.Normalize();
					
					//  camera to hit
					camera_vector = Vector3(ray.org_x - ray.dir_x, ray.org_y - ray.dir_y, ray.org_z - ray.dir_z);
					camera_vector.Normalize();
					
					gamma = material->shininess;
					i_d_r = light.diffuse_.z;
					i_d_g = light.diffuse_.y;
					i_d_b = light.diffuse_.x;
					i_s_r = light.spectular_.z;
					i_s_g = light.spectular_.y;
					i_s_b = light.spectular_.x;
					m_s_r = material->specular.z;
					m_s_g = material->specular.y;
					m_s_b = material->specular.x;

					//Vector3 l_r{ -light_vector.x, -light_vector.y, light_vector.z };
					Vector3 l_r = 2 * (normal_vector.DotProduct(light_vector)) * normal_vector - light_vector;
					l_r.Normalize();

					blue += (i_d_b * m_d_b * (normal_vector.DotProduct(light_vector)) 
						+ i_s_b * m_s_b * (pow(camera_vector.DotProduct(l_r), gamma)));
					green += (i_d_g * m_d_g * (normal_vector.DotProduct(light_vector)) 
						+ i_s_g * m_s_g * (pow(camera_vector.DotProduct(l_r), gamma)));
					red += (i_d_r * m_d_r * (normal_vector.DotProduct(light_vector)) 
						+ i_s_r * m_s_r * (pow(camera_vector.DotProduct(l_r), gamma)));
				}
			}
			return Color4f{ blue, green , red , 1.0f };
		}

	}
	// pokud se nic nehitlo, vratim pozadi
	return this->background_.get_texel(
		direction_vector.x,
		direction_vector.y,
		direction_vector.z);
}

Color4f Raytracer::get_pixel( const int x, const int y, const float t )
{
	const int ms_width = 5;
	int ms_total = pow(ms_width, 2);

	//std::array<std::array<Color4f, ms_width>, ms_width> result_colors;
	Color4f result_colors[ms_width][ms_width];
	// generate primary ray and perform ray cast on the scene
	// TODO supersampling
	if (ms_total == 1) {
		RTCRay primary_ray = camera_.GenerateRay(x, y);
		primary_ray.time = IOR_AIR;
		return gamma(trace(primary_ray, 0));
	}
	for (int fieldX = 0; fieldX < ms_width; fieldX++) {

		float msX = fieldX * (1.0f / ms_width);
		for (int fieldY = 0; fieldY < ms_width; fieldY++) {
			float msY = fieldY * (1.0f / ms_width);
			std::mt19937 generator(123);
			std::uniform_real_distribution<float> uni_dist(-0.5f / ms_width, 0.5f / ms_width);
			float rand1 = uni_dist(generator);
			float rand2 = uni_dist(generator);
		
			RTCRay primary_ray = camera_.GenerateRay(x + msX + rand1, y + msY + rand2, this->focalDistance, this->apertureSize);
			primary_ray.time = IOR_AIR;
			result_colors[fieldX][fieldY] = trace(primary_ray, 0);
		}

	}
	Color4f tmpMultisamplingColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	for (int fieldX = 0; fieldX < ms_width; fieldX++) {
		for (int fieldY = 0; fieldY < ms_width; fieldY++) {
			tmpMultisamplingColor.r += result_colors[fieldX][fieldY].r;
			tmpMultisamplingColor.g += result_colors[fieldX][fieldY].g;
			tmpMultisamplingColor.b += result_colors[fieldX][fieldY].b;
		}
	}
	return Color4f{ tmpMultisamplingColor.b/ ms_total, tmpMultisamplingColor.g /ms_total, tmpMultisamplingColor.r / ms_total, 1.0f };

}

Color4f Raytracer::gamma(Color4f input) {
	// Q10 Gamma correction http://mrl.cs.vsb.cz/people/fabian/pg1/pr02.pdf
	float b = pow(input.b, gamma_level) * pow(input.b, gamma_level);
	float g = pow(input.g, gamma_level) * pow(input.g, gamma_level);
	float r = pow(input.r, gamma_level) * pow(input.r, gamma_level);

	return Color4f{ b, g, r, 1.0f };
}

int Raytracer::Ui()
{
	static float f = 0.5f;
	static int counter = 0;

	// Use a Begin/End pair to created a named window
	ImGui::Begin( "Ray Tracer Params" );
	
	ImGui::Text( "Surfaces = %d", surfaces_.size() );
	ImGui::Text( "Materials = %d", materials_.size() );
	ImGui::Separator();
	ImGui::Checkbox( "Vsync", &vsync_ );
	
	//ImGui::Checkbox( "Demo Window", &show_demo_window ); // Edit bools storing our window open/close state
	//ImGui::Checkbox( "Another Window", &show_another_window );

	ImGui::SliderFloat( "gamma", &f, 0.0f, 1.0f ); // Edit 1 float using a slider from 0.0f to 1.0f 
	gamma_level = f;

	ImGui::Text("counter = %d", counter);

	ImGui::SliderFloat("aperture", &this->apertureSize, 0.0f, 5.0f);

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


