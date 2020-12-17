#include "stdafx.h"
#include "texture.h"
#include <iostream>
Texture::Texture() { }
Texture::Texture( const char * file_name )
{
	// image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// pointer to the image, once loaded
	FIBITMAP * dib =  nullptr;
	// pointer to the image data
	BYTE * bits = nullptr;

	// check the file signature and deduce its format
	fif = FreeImage_GetFileType( file_name, 0 );
	// if still unknown, try to guess the file format from the file extension
	if ( fif == FIF_UNKNOWN )
	{
		fif = FreeImage_GetFIFFromFilename( file_name );
	}
	// if known
	if ( fif != FIF_UNKNOWN )
	{
		// check that the plugin has reading capabilities and load the file
		if ( FreeImage_FIFSupportsReading( fif ) )
		{
			dib = FreeImage_Load( fif, file_name );
		}
		// if the image loaded
		if ( dib )
		{
			// retrieve the image data
			bits = FreeImage_GetBits( dib );
			//FreeImage_ConvertToRawBits()
			// get the image width and height
			width_ = int( FreeImage_GetWidth( dib ) );
			height_ = int( FreeImage_GetHeight( dib ) );

			// if each of these is ok
			if ( ( bits != 0 ) && ( width_ != 0 ) && ( height_ != 0 ) )
			{				
				// texture loaded
				scan_width_ = FreeImage_GetPitch( dib ); // in bytes
				pixel_size_ = FreeImage_GetBPP( dib ) / 8; // in bytes

				data_ = new BYTE[scan_width_ * height_]; // BGR(A) format
				FreeImage_ConvertToRawBits( data_, dib, scan_width_, pixel_size_ * 8, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE );
			}

			FreeImage_Unload( dib );
			bits = nullptr;
		}
	}	
}

Color3f Texture::get_pixel(int x, int y) {
	int offset = y * scan_width_ + x * pixel_size_;
	float b = data_[offset] / 255.0f;
	float g = data_[offset + 1] / 255.0f;
	float r = data_[offset + 2] / 255.0f;
	return Color3f{ r, g, b };
};

Texture::~Texture()
{	
	/*if ( data_ )
	{
		// free FreeImage's copy of the data
		delete[] data_;
		data_ = nullptr;
		
		width_ = 0;
		height_ = 0;
	}*/
}

Color3f Texture::get_texel( const float u, const float v )
{
	float x, y, Q11, Q21, Q12, Q22,final_r, final_g, final_b;
	Color3f x1y1, x2y1, x1y2, x2y2, f_xy1, f_xy2;
	int x1, x2, y1, y2;

	x = u * float(width_);
	y = v * float(height_);

	x1 = floor(x);
	x2 = ceil(x);
	y1 = floor(y);
	y2 = ceil(y);

	if (x2 >= width_)x2 = 0.0f;
	if (y2 >= height_)y2 = y1;
	if (x1 == x2 && x1 != 0)x1--; else x2++;
	if (y1 == y2 && y1 != 0)y1--; else y2++;

	x1y1 = get_pixel(x1, y1);
	x2y1 = get_pixel(x2, y1);
	x1y2 = get_pixel(x1, y2);
	x2y2 = get_pixel(x2, y2);

	Q11 = float((x2 - x) / (x2 - x1));
	Q21 = float((x - x1) / (x2 - x1));
	Q12 = float((x2 - x) / (x2 - x1));
	Q22 = float((x - x1) / (x2 - x1));

	f_xy1 = Color3f{ 
		x1y1.r * Q11 + x2y1.r * Q21, 
		x1y1.g * Q11 + x2y1.g * Q21, 
		x1y1.b * Q11 + x2y1.b * Q21 };

	f_xy2 = Color3f{ 
		x1y2.r * Q12 + x2y2.r * Q22, 
		x1y2.g * Q12 + x2y2.g * Q22, 
		x1y2.b * Q12 + x2y2.b * Q22 };
	
	final_r = f_xy1.r * ((y2 - y) / (y2 - y1)) + f_xy2.r * ((y - y1) / (y2 - y1));
	final_g = f_xy1.g * ((y2 - y) / (y2 - y1)) + f_xy2.g * ((y - y1) / (y2 - y1));
	final_b = f_xy1.b * ((y2 - y) / (y2 - y1)) + f_xy2.b * ((y - y1) / (y2 - y1));
	
	return Color3f{ final_r, final_g, final_b };
}

int Texture::width() const
{
	return width_;
}

int Texture::height() const
{
	return height_;
}


