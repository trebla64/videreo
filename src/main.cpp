#include "stdafx.h"
#include "vec3.h"
#include "vec2.h"
#include "sphere.h"
#include "scene.h"

#pragma comment(lib, "embree3.lib")

static const int SCREEN_WIDTH  = 1024;
static const int SCREEN_HEIGHT = 768;
#ifndef M_PI
static const float M_PI = 3.1415926535897932384626433832795f;
#endif
static const float kFOV = 30.0f;
static const int NUM_SAMPLES = 1;	// Increase this for anti-aliasing, 1 is preview mode

static uint8_t _buffer[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

static unsigned gIntersectionTests = 0;
static unsigned gHits = 0;

#if 0
vec3 worldApplyLighting( in vec3 pos, in vec3 nor )
{
     vec3 dcol = vec3(0.0);

     // sample sun
     {
     vec3  point = 1000.0*sunDirection + 50.0*diskPoint(nor);
     vec3  liray = normalize( point - pos );
     float ndl =  max(0.0, dot(liray, nor));
     dcol += ndl * sunColor * worldShadow( pos, liray );;
     }

     // sample sky
     {
     vec3  point = 1000.0*cosineDirection(nor);
     vec3  liray = normalize( point - pos );
     dcol += skyColor * worldShadow( pos, liray );
     }

     return dcol;
}

//----------------------------------
// Most basic implementation
// run for every pixel in the image
//----------------------------------
vec3 calcPixelColor( in vec2 pixel, in vec2 resolution, in float frameTime )
{
    // screen coords
    vec2 p = (-resolution + 2.0*pixel) / resolution.y;

    // move objects
    worldMoveObjects( frameTime );

    // get camera position, and right/up/front axis
    vec3 (ro, uu, vv, ww) = worldMoveCamera( frameTime );

    // create ray
    vec3 rd = normalize( p.x*uu + p.y*vv + 2.5*ww );

    // calc pixel color
    vec3 col = rendererCalculateColor( ro, rd );

    // apply gamma correction
    col = pow( col, 0.45 );

    return col;
}

//----------------------------------
// Implementation with sampling
// run for every pixel in the image
//----------------------------------
vec3 calcPixelColor( in vec2 pixel, in vec2 resolution, in float frameTime )
{
    float shutterAperture = 0.6;
    float fov = 2.5;
    float focusDistance = 1.3;
    float blurAmount = 0.0015;
    int   numLevels = 5;

    // 256 paths per pixel
    vec3 col = vec3(0.0);
    for( int i=0; i<256; i++ )
    {
        // screen coords with antialiasing
        vec2 p = (-resolution + 2.0*(pixel + random2f())) / resolution.y;

        // motion blur
        float ctime = frameTime + shutterAperture*(1.0/24.0)*random1f();

        // move objects
        worldMoveObjects( ctime );

        // get camera position, and right/up/front axis
        vec3 (ro, uu, vv, ww) = worldMoveCamera( ctime );

        // create ray with depth of field
        vec3 er = normalize( vec3( p.xy, fov ) );
        vec3 rd = er.x*uu + er.y*vv + er.z*ww;

        vec3 go = blurAmount*vec3( -1.0 + 2.0*random2f(), 0.0 );
        vec3 gd = normalize( er*focusDistance - go );
        ro += go.x*uu + go.y*vv;
        rd += gd.x*uu + gd.y*vv;

        // accumulate path
        col += rendererCalculateColor( ro, normalize(rd), numLevels );
    }
    col = col / 256.0;

    // apply gamma correction
    col = pow( col, 0.45 );

    return col;
}

//Camera code
mat3 setCamera(in vec3 ro, in vec3 rt, in float cr)
{
	vec3 cw = normalize(rt - ro);
	vec3 cp = vec3(sin(cr), cos(cr), 0.0);
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = normalize(cross(cu, cw));
	return mat3(cu, cv, -cw);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	float sa = hash(dot(fragCoord, vec2(12.9898, 78.233)) + 1113.1 * float(iFrame));

	vec2 of = -0.5 + vec2(hash(sa + 13.271), hash(sa + 63.216));
	vec2 p = (-iResolution.xy + 2.0 * (fragCoord + of)) / iResolution.y;

	vec3 ro = vec3(0.0, 0.0, 0.0);
	vec3 ta = vec3(1.5, 0.7, 1.5);

	mat3  ca = setCamera(ro, ta, 0.0);
	vec3  rd = normalize(ca * vec3(p, -1.3));
#endif

static vec2 random2f()
{
	vec2 res;

	res.x = float(rand()) / float(RAND_MAX);
	res.y = float(rand()) / float(RAND_MAX);

	return res;
}

static void EmbreeErrorFunction(void* userPtr, enum RTCError code, const char* str)
{
	FILE *flog = (FILE *)userPtr;

	fprintf(flog, "[Embree] %s\n", str);
}

static void TestLoadModel()
{
	std::cout << "TestLoadModel" << std::endl;

	tinyply::PlyFile ply;
	std::ifstream ss("D:\\Repos\\pbrt-v3-scenes\\dragon\\geometry\\dragon_remeshed.ply", std::ios::binary);
	if (ss.fail())
	{
		std::cout << "Failed loading file!" << std::endl;
	}

	ply.parse_header(ss);

	for (auto c : ply.get_comments()) std::cout << "Comment: " << c << std::endl;

	for (auto e : ply.get_elements())
	{
		std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
		for (auto p : e.properties)
		{
			std::cout << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
		}
	}
}

int main(int argc, char *argv[])
{
	TestLoadModel();

	// Parse arguments
	if (argc != 2) {
		printf("Usage: %s <scene file>\n", argv[0]);
		return EXIT_SUCCESS;
	}

	const char *scene_file = argv[1];

	// Logging
	FILE *flog = fopen("output.log", "wt");
	if (!flog) {
		fprintf(stderr, "Error: Failed opening log file\n");
		return EXIT_FAILURE;
	}

	RTCDevice dev = rtcNewDevice(nullptr);
	rtcSetDeviceErrorFunction(dev, EmbreeErrorFunction, flog);
	Scene newScene(dev);
	newScene.LoadFromFile(scene_file);

	newScene.Commit();

	vec2 resolution((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

	printf("Starting trace...\n");

	uint8_t *pbuf = _buffer;
	for (unsigned y = 0; y < SCREEN_HEIGHT; y++) {
		for (unsigned x = 0; x < SCREEN_WIDTH; x++) {
			vec2 pixel((float)x, (float)y);

			vec3 col(0.0f, 0.0f, 0.0f);
			for (int samples = 0; samples < NUM_SAMPLES; samples++) {
				vec2 jittered_pixel(0.0f, 0.0f);
				if (NUM_SAMPLES > 1)
					jittered_pixel = random2f();
				vec2 p = (-resolution + 2.0f * (pixel + jittered_pixel)) / resolution.y;
				vec3 raydir(p.x, p.y, -2.5f);
				raydir.normalize();

				RTCRayHit rayhit;
				rayhit.ray.flags = 0;

				rayhit.ray.org_x = 0.0f;
				rayhit.ray.org_y = 0.0f;
				rayhit.ray.org_z = 0.0f;
				rayhit.ray.dir_x = raydir.x;
				rayhit.ray.dir_y = raydir.y;
				rayhit.ray.dir_z = raydir.z;
				rayhit.ray.tnear = 0.0f;
				rayhit.ray.tfar = 1000.0f;
				rayhit.ray.time = 0.0f;
				rayhit.ray.mask = 0xFFFFFFFF;

				rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
				rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;
				rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

				RTCIntersectContext ctx;
				rtcInitIntersectContext(&ctx);
				newScene.Intersect1(&ctx, &rayhit);

				Primitive *prim = newScene.FindByGeomID(rayhit.hit.geomID);
				if (prim) {
					col += vec3(1.0f, 1.0f, 1.0f);
				}
			}
			col /= float(NUM_SAMPLES);

			// Gamma correction
			col.pow(0.45f);

			col *= 255.0f;
			*pbuf++ = (uint8_t)col.x;
			*pbuf++ = (uint8_t)col.y;
			*pbuf++ = (uint8_t)col.z;
		}
	}

	printf("Done tracing!\n");
	fprintf(stderr, "Intersection tests: %u, Hits = %u\n", gIntersectionTests, gHits);

	FILE *fp = fopen("out.ppm", "wb");
	if (fp) {
		fprintf(fp, "P6\n%d %d\n255\n", SCREEN_WIDTH, SCREEN_HEIGHT);
		fwrite(_buffer, sizeof(_buffer), 1, fp);
		fflush(fp);
		fclose(fp);
	}

	rtcReleaseDevice(dev);

	fflush(flog);
	fclose(flog);

	return EXIT_SUCCESS;
}
