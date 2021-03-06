#include "stdafx.h"
#include "tutorials.h"

int main()
{
	printf( "PG1, (c)2011-2020 Tomas Fabian\n\n" );

	_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );
	/* //return tutorial_1();
	//return tutorial_2();
	//return tutorial_3( "../../../data/6887_allied_avenger.obj" ); */

	const std::string obj_avenger_path = "../../../data/6887_allied_avenger.obj";
	//const std::string obj_sphere_path = "../../../data/sphere/geosphere.obj";
	//const std::string bg_windows_path = "../../../data/spherical_map_windows.jpg";
	//const std::string bg_haus_path = "../../../data/spherical_map_haus.jpg";
	const std::string bg_lake_path = "../../../data/spherical_map_lakeside.jpg";
	//const std::string bg_rails_path = "../../../data/spherical_map_rails.jpg";

	return raytrace_loop( obj_avenger_path, bg_lake_path );
}
