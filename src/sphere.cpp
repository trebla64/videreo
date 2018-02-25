#include "stdafx.h"
#include "sphere.h"

void SphereObjectBounds(const struct RTCBoundsFunctionArguments* args)
{
	const Sphere *obj = (const Sphere *)args->geometryUserPtr;
	RTCBounds *bounds = args->bounds_o;
	bounds->lower_x = obj->pos.x - obj->radius;
	bounds->lower_y = obj->pos.y - obj->radius;
	bounds->lower_z = obj->pos.z - obj->radius;
	bounds->upper_x = obj->pos.x + obj->radius;
	bounds->upper_y = obj->pos.y + obj->radius;
	bounds->upper_z = obj->pos.z + obj->radius;
}

void SphereObjectIntersect(const struct RTCIntersectFunctionNArguments* args)
{
	unsigned int N_in = args->N;
	assert(N_in == 1);
	void* valid_in = args->valid;
	int* vvalid_in = (int*)valid_in;
	if (*vvalid_in != -1)
		return;

	RTCIntersectContext* context_in = args->context;
	unsigned int i = args->primID;
	const Sphere *obj = (const Sphere *)args->geometryUserPtr;

	RTCRayHitN *rayhit = args->rayhit;
	RTCRayN *ray = RTCRayHitN_RayN(rayhit, 1);
	RTCHitN *hit = RTCRayHitN_HitN(rayhit, 1);

	float org_x = RTCRayN_org_x(ray, 1, 0);
	float org_y = RTCRayN_org_y(ray, 1, 0);
	float org_z = RTCRayN_org_z(ray, 1, 0);

	float dir_x = RTCRayN_dir_x(ray, 1, 0);
	float dir_y = RTCRayN_dir_y(ray, 1, 0);
	float dir_z = RTCRayN_dir_z(ray, 1, 0);

	float &tfar = RTCRayN_tfar(ray, 1, 0);

	unsigned &geomID = RTCHitN_geomID(hit, 1, 0);
	unsigned &primID = RTCHitN_primID(hit, 1, 0);
	unsigned &instID = RTCHitN_instID(hit, 1, 0, 0);

	vec3 l;
	l.x = obj->pos.x - org_x;
	l.y = obj->pos.y - org_y;
	l.z = obj->pos.z - org_z;
	float tca = (l.x * dir_x) + (l.y * dir_y) + (l.z * dir_z);
	if (tca < 0.0f) return;
	float d2 = l * l - tca * tca;
	if (d2 > obj->radius2) return;

	// Hit
	float thc = sqrtf(obj->radius2 - d2);
	float t = tca - thc;
	float t1 = tca + thc;
	if (t1 < t) t = t1;

	tfar = t;
	geomID = i;
	primID = 0;
	instID = context_in->instID[0];
}

void SphereObjectOccluded(const struct RTCOccludedFunctionNArguments* args)
{
	const Sphere *obj = (const Sphere *)args->geometryUserPtr;

#if 0
	// TODO: Fix this
	vec3 l;
	l.x = obj->pos.x - ray.org_x;
	l.y = obj->pos.y - ray.org_y;
	l.z = obj->pos.z - ray.org_z;
	float tca = (l.x * ray.dir_x) + (l.y * ray.dir_y) + (l.z * ray.dir_z);
	if (tca < 0.0f) return;
	float d2 = l * l - tca * tca;
	if (d2 > obj->radius2) return;

	ray.geomID = 0;
#endif
}