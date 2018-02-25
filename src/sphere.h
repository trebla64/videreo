#pragma once

#include "primitive.h"
#include "vec3.h"

class Sphere : public Primitive
{
public:
	vec3  pos;
	float radius;
	float radius2;
	unsigned geomID;

public:
	Sphere() = delete;
	Sphere(const vec3 &pos_, float radius_) : pos(pos_), radius(radius_), radius2(radius_*radius_) {}

	virtual PrimitiveType GetType() const { return PT_SPHERE; }
	virtual unsigned      GetGeomID() const { return geomID; }
	virtual void          SetGeomID(unsigned geomID) { this->geomID = geomID; }
};

// TODO: Move this into Sphere class as static methods
void SphereObjectBounds(const struct RTCBoundsFunctionArguments* args);
void SphereObjectIntersect(const struct RTCIntersectFunctionNArguments* args);
void SphereObjectOccluded(const struct RTCOccludedFunctionNArguments* args);