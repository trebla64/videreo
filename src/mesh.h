#pragma once

#include "primitive.h"

class Mesh : public Primitive
{
public:
	Mesh() = delete;
	Mesh(const std::string &source_) : source(source_) {}

public:
	virtual PrimitiveType GetType() const { return PT_MESH; }
	virtual unsigned      GetGeomID() const { return geomID; }	// For Embree
	virtual void          SetGeomID(unsigned geomID) { this->geomID = geomID; }	// For Embree

private:
	std::string source;
	unsigned    geomID;
};

// TODO: Move this into Mesh class as static methods
void MeshObjectBounds(const struct RTCBoundsFunctionArguments* args);
void MeshObjectIntersect(const struct RTCIntersectFunctionNArguments* args);
void MeshObjectOccluded(const struct RTCOccludedFunctionNArguments* args);