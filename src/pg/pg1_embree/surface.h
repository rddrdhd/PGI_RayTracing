#ifndef SURFACE_H_
#define SURFACE_H_

#include "vertex.h"
#include "material.h"
#include "triangle.h"

/*! \class Surface
\brief A class representing a triangular mesh.

\author Tomáš Fabián
\version 0.9
\date 2012-2018
*/
class Surface
{
public:
	//! Vıchozí konstruktor.
	/*!
	Inicializuje všechny sloky sítì na hodnotu nula.
	*/
	Surface();

	//! Obecnı konstruktor.
	/*!
	Inicializuje vertex podle zadanıch hodnot parametrù.

	\param name název plochy.
	\param n poèet trojúhelníkù tvoøících sí.
	*/
	Surface( const std::string & name, const int n );

	//! Destruktor.
	/*!
	Uvolní všechny alokované zdroje.
	*/
	~Surface();	

	//! Vrátí poadovanı trojúhelník.
	/*!
	\param i index trojúhelníka.
	\return Trojúhelník.
	*/
	Triangle & get_triangle( const int i );

	//! Vrátí pole všech trojúhelníkù.
	/*!	
	\return Pole všech trojúhelníkù.
	*/
	Triangle * get_triangles();

	//! Vrátí název plochy.
	/*!	
	\return Název plochy.
	*/
	std::string get_name();

	//! Vrátí poèet všech trojúhelníkù v síti.
	/*!	
	\return Poèet všech trojúhelníkù v síti.
	*/
	int no_triangles();

	//! Vrátí poèet všech vrcholù v síti.
	/*!	
	\return Poèet všech vrcholù v síti.
	*/
	int no_vertices();		
	
	//! Nastaví materiál plochy.
	/*!	
	\param material ukazatel na materiál.
	*/
	void set_material( Material * material );

	//! Vrátí ukazatel na materiál plochy.
	/*!	
	\return Ukazatel na materiál plochy.
	*/
	Material * get_material() const;

protected:

private:
	int n_{ 0 }; /*!< Poèet trojúhelníkù v síti. */
	Triangle * triangles_{ nullptr }; /*!< Trojúhelníková sí. */

	std::string name_{ "unknown" }; /*!< Název plochy. */

	//Matrix4x4 transformation_; /*!< Transformaèní matice pro pøechod z modelového do svìtového souøadného systému. */
	Material * material_{ nullptr }; /*!< Materiál plochy. */
};

/*! \fn Surface * BuildSurface( const std::string & name, std::vector<Vertex> & face_vertices )
\brief Sestavení plochy z pole trojic vrcholù.
\param name název plochy.
\param face_vertices pole trojic vrcholù.
*/
Surface * BuildSurface( const std::string & name, std::vector<Vertex> & face_vertices );

#endif
