#pragma once

#include "primitive.h"

class Scene
{
private:
	RTCScene scene;
	RTCDevice _device;
	std::vector<std::unique_ptr<Primitive>> primitives;

public:
	Scene() = delete;
	Scene(RTCDevice device);
	~Scene();

	bool      LoadFromFile(const char *filename);
	Primitive *FindByGeomID(unsigned geomID);

	// Embree interfaces
	unsigned  AttachGeometry(RTCGeometry geom);
	void      Commit();
	void      Intersect1(RTCIntersectContext* context, RTCRayHit* rayhit);
};