#include "stdafx.h"
#include "scene.h"
#include "sphere.h"
#include "mesh.h"

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
				rtcReleaseGeometry(geom);
				primitive->SetGeomID(geomID);

				// https://stackoverflow.com/questions/3283778/why-can-i-not-push-back-a-unique-ptr-into-a-vector
				this->primitives.push_back(std::move(primitive));
			}
			else if (type == "mesh") {
				assert(item.HasMember("source"));
				assert(item["source"].IsString());

				// Try loading what is set as a source as PLY model

				auto mesh = new Mesh(item["source"].GetString());

				// TODO: Use mesh loading code here instead of assuming PLY file by default
				//mesh->Load();

				std::ifstream ss(item["source"].GetString(), std::ios::binary);
				if (ss.fail())
				{
					std::cout << "Failed loading file!" << std::endl;
				}

				tinyply::PlyFile ply;
				ply.parse_header(ss);
				
				std::shared_ptr<tinyply::PlyData> vertices, faces;
				vertices = ply.request_properties_from_element("vertex", { "x", "y", "z" });
				size_t vertex_count = 0;
				if (vertices)
					vertex_count = vertices->count;

				faces = ply.request_properties_from_element("face", { "vertex_indices" });
				size_t face_count = 0;
				if (faces)
					face_count = faces->count;

				// TODO: Use make_unique here
				std::unique_ptr<Primitive> primitive = std::unique_ptr<Mesh>(mesh);

				RTCGeometry geom = rtcNewGeometry(_device, RTC_GEOMETRY_TYPE_TRIANGLE);

				ply.read(ss);
				
				struct Vertex {
					float x; float y; float z;
				};
				const size_t numVerticesBytes = vertices->buffer.size_bytes();
				Vertex *dst_vertices = (Vertex*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), vertex_count);
				std::memcpy(dst_vertices, vertices->buffer.get(), numVerticesBytes);

				const size_t numFacesBytes = faces->buffer.size_bytes();
				const size_t bytesPerFace = numFacesBytes / face_count;
				struct Triangle {
					uint32_t i0; uint32_t i1; uint32_t i2;
				};
				Triangle *dst_faces = (Triangle*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), face_count);
				std::memcpy(dst_faces, faces->buffer.get(), numFacesBytes);

				rtcCommitGeometry(geom);

#if 0
				/* create a triangulated plane with 2 triangles and 4 vertices */
				RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

				/* set vertices */
				Vertex* vertices = (Vertex*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 4);
				vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
				vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
				vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
				vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

				/* set triangles */
				Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2);
				triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
				triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;

				rtcCommitGeometry(mesh);
				unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
				rtcReleaseGeometry(mesh);
				return geomID;
#endif

				unsigned geomID = AttachGeometry(geom);
				rtcReleaseGeometry(geom);
				primitive->SetGeomID(geomID);

				this->primitives.push_back(std::move(primitive));
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