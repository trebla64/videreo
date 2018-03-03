#include "stdafx.h"
#include "scene.h"
#include "sphere.h"

Scene::Scene(RTCDevice device)
{
	_device = device;
	scene = rtcNewScene(_device);
	rtcSetSceneFlags(scene, RTC_SCENE_FLAG_NONE | RTC_SCENE_FLAG_ROBUST);
	rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_MEDIUM);
}

Scene::~Scene()
{
	// TODO: Release reference to device?
	rtcReleaseScene(scene);
}

unsigned Scene::AttachGeometry(RTCGeometry geom)
{
	return rtcAttachGeometry(scene, geom);
}

void Scene::Commit()
{
	rtcCommitScene(scene);
}

void Scene::Intersect1(RTCIntersectContext* context, RTCRayHit* rayhit)
{
	rtcIntersect1(scene, context, rayhit);
}

bool Scene::LoadFromFile(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		unsigned size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		uint8_t *buffer = (uint8_t *)malloc(size + 1);
		if (!buffer) {
			fclose(fp);
			return false;
		}
		// TODO: Check that output size is correct
		fread(buffer, size, 1, fp);
		buffer[size] = '\0';

		// Actually parse the JSON
		rapidjson::Document d;
		if (d.ParseInsitu((char *)buffer).HasParseError()) {
			free(buffer);
			fclose(fp);
			return false;
		}

		// TODO: Replace asserts with proper validation and logging
		assert(d.IsObject());
		assert(d.HasMember("primitives"));
		assert(d["primitives"].IsArray());

		const rapidjson::Value &primitives = d["primitives"];
		for (rapidjson::Value::ConstValueIterator it = primitives.Begin(); it != primitives.End(); it++) {
			const rapidjson::Value &item = *it;

			assert(item.IsObject());
			assert(item.HasMember("type"));
			assert(item["type"].IsString());

			std::string type = item["type"].GetString();
			if (type == "sphere") {
				assert(item.HasMember("position"));
				assert(item["position"].IsArray());
				assert(item.HasMember("radius"));
				assert(item["radius"].IsNumber());

				float pos[3];

				unsigned vector_elems = 0;
				const rapidjson::Value &position = item["position"];
				for (rapidjson::Value::ConstValueIterator it1 = position.Begin(); it1 != position.End(); it1++) {
					if (vector_elems >= 3)
						assert(0 && "Too many vector elements");

					const rapidjson::Value &elem = *it1;
					assert(elem.IsNumber());

					pos[vector_elems] = elem.GetFloat();

					vector_elems++;
				}

				float radius = item["radius"].GetFloat();

				// Add sphere to Embree scene
				auto sphere = new Sphere(vec3(pos[0], pos[1], pos[2]), radius);
				// TODO: Use make_unique here
				std::unique_ptr<Primitive> primitive = std::unique_ptr<Sphere>(sphere);

				RTCGeometry geom = rtcNewGeometry(_device, RTC_GEOMETRY_TYPE_USER);
				rtcSetGeometryBuildQuality(geom, RTC_BUILD_QUALITY_MEDIUM);
				rtcSetGeometryUserPrimitiveCount(geom, 1);
				rtcSetGeometryTimeStepCount(geom, 1);
				rtcSetGeometryUserData(geom, sphere);
				rtcSetGeometryBoundsFunction(geom, SphereObjectBounds, nullptr);
				rtcSetGeometryIntersectFunction(geom, SphereObjectIntersect);
				rtcSetGeometryOccludedFunction(geom, SphereObjectOccluded);
				rtcCommitGeometry(geom);

				// Attach geometry to Embree scene and save geomID
				unsigned geomID = AttachGeometry(geom);
				primitive->SetGeomID(geomID);

				// https://stackoverflow.com/questions/3283778/why-can-i-not-push-back-a-unique-ptr-into-a-vector
				this->primitives.push_back(std::move(primitive));
			}
			else if (type == "mesh") {
			}
			else {
				// Invalid primitive type
				assert(0 && "Invalid primitive type in scene file");
			}
		}

		free(buffer);
		fclose(fp);
	}
	else {
		return false;
	}

	return true;
}

// Returns nullptr when nothing found
Primitive *Scene::FindByGeomID(unsigned geomID)
{
	for (const std::unique_ptr<Primitive> &p : this->primitives)
		if (geomID == p->GetGeomID())
			return p.get();

	return nullptr;
}