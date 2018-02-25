#pragma once

struct vec3
{
	float x, y, z;

	vec3() {}
	vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	float length2() const { return (x*x + y * y + z * z); }
	float length() const { return sqrtf(length2()); }

	vec3 operator - (const vec3 &rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	vec3 operator - (const float *rhs) const { return vec3(x - rhs[0], y - rhs[1], z - rhs[2]); }

	float operator * (const vec3 &rhs) const { return (x*rhs.x + y * rhs.y + z * rhs.z); }
	float operator * (const float *rhs) const { return (x*rhs[0] + y * rhs[1] + z * rhs[2]); }

	vec3 &operator += (const vec3 &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3 &operator *= (float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
	vec3 &operator /= (float rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

	void normalize()
	{
		float invlen = 1.0f / length();
		x *= invlen;
		y *= invlen;
		z *= invlen;
	}

	void pow(float a)
	{
		x = powf(x, a);
		y = powf(y, a);
		z = powf(z, a);
	}
};