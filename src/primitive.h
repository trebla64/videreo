#pragma once

enum PrimitiveType {
	PT_NULL = 0,
	PT_SPHERE,
	PT_MESH
};

class Primitive
{
public:
	virtual PrimitiveType GetType() const = 0;
	virtual unsigned      GetGeomID() const = 0;	// For Embree
	virtual void          SetGeomID(unsigned geomID) = 0;	// For Embree
};