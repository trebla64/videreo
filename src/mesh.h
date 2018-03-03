#pragma once

#include "primitive.h"

class Mesh : public Primitive
{
public:
	Mesh() = delete;
	Mesh(const std::string &source_) : source(source_) {}

private:
	std::string  source;
};

// TODO: Move this into Mesh class as static methods
void MeshObjectBounds(const struct RTCBoundsFunctionArguments* args);
void MeshObjectIntersect(const struct RTCIntersectFunctionNArguments* args);
void MeshObjectOccluded(const struct RTCOccludedFunctionNArguments* args);