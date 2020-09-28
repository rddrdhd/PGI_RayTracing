#include "stdafx.h"
#include "matrix3x3.h"

Matrix3x3::Matrix3x3()
{
	for ( int r = 0; r < 3; ++r )
	{
		for ( int c = 0; c < 3; ++c )
		{
			data_[c + r * 3] = ( ( r == c ) ? 1.0f : 0.0f );
		}
	}
}

Matrix3x3::Matrix3x3( const float m00, const float m01, const float m02,
		const float m10, const float m11, const float m12,
		const float m20, const float m21, const float m22 )
{
	m00_ = m00;
	m01_ = m01;
	m02_ = m02;

	m10_ = m10;
	m11_ = m11;
	m12_ = m12;	

	m20_ = m20;
	m21_ = m21;
	m22_ = m22;
}

Matrix3x3::Matrix3x3( const Vector3 basis_x, const Vector3 basis_y, const Vector3 basis_z )
{
	m00_ = basis_x.x;
	m01_ = basis_y.x;
	m02_ = basis_z.x;

	m10_ = basis_x.y;
	m11_ = basis_y.y;
	m12_ = basis_z.y;

	m20_ = basis_x.z;
	m21_ = basis_y.z;
	m22_ = basis_z.z;
}

Matrix3x3 Matrix3x3::Transpose() const
{	
	return Matrix3x3( m00_, m10_, m20_,
		m01_, m11_, m21_,
		m02_, m12_, m22_ );
}

void Matrix3x3::set( const int row, const int column, const float value )
{
	assert( row >= 0 && row < 3 && column >= 0 && column < 3 );

	data_[column + row * 3] = value;
}

float Matrix3x3::get( const int row, const int column ) const
{
	assert( row >= 0 && row < 3 && column >= 0 && column < 3 );

	return data_[column + row * 3];
}

Vector3 operator*( const Matrix3x3 & a, const Vector3 & b )
{
	return Vector3( a.m00_ * b.x + a.m01_ * b.y + a.m02_ * b.z,
		a.m10_ * b.x + a.m11_ * b.y + a.m12_ * b.z,
		a.m20_ * b.x + a.m21_ * b.y + a.m22_ * b.z );
}

Matrix3x3 operator*( const Matrix3x3 & a, const Matrix3x3 & b )
{
	return Matrix3x3( a.m00_ * b.m00_ + a.m01_ * b.m10_ + a.m02_ * b.m20_,
		a.m00_ * b.m01_ + a.m01_ * b.m11_ + a.m02_ * b.m21_,
		a.m00_ * b.m02_ + a.m01_ * b.m12_ + a.m02_ * b.m22_,		

		a.m10_ * b.m00_ + a.m11_ * b.m10_ + a.m12_ * b.m20_,
		a.m10_ * b.m01_ + a.m11_ * b.m11_ + a.m12_ * b.m21_,
		a.m10_ * b.m02_ + a.m11_ * b.m12_ + a.m12_ * b.m22_,		

		a.m20_ * b.m00_ + a.m21_ * b.m10_ + a.m22_ * b.m20_,
		a.m20_ * b.m01_ + a.m21_ * b.m11_ + a.m22_ * b.m21_,
		a.m20_ * b.m02_ + a.m21_ * b.m12_ + a.m22_ * b.m22_ );
}
