#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "freeimage.h"
#include "structs.h"

/*! \class Texture
\brief Single texture.

\author Tom� Fabi�n
\version 0.95
\date 2012-2018
*/
class Texture
{
public:
	Texture();
	Texture( const char * file_name );
	Color3f get_pixel(int x, int y);
	~Texture();

	Color3f get_texel(const float u, const float v);

	int width() const;
	int height() const;

private:	
	int width_{ 0 }; // image width (px)
	int height_{ 0 }; // image height (px)
	int scan_width_{ 0 }; // size of image row (bytes)
	int pixel_size_{ 0 }; // size of each pixel (bytes)
	BYTE * data_{ nullptr }; // image data in BGR format
};

#endif
