#pragma once

struct vec2
{
	float x, y;

	vec2() {}
	vec2(float x_, float y_) : x(x_), y(y_) {}

	vec2 operator + (const vec2 &rhs) const { return vec2(x + rhs.x, y + rhs.y); }

	vec2 operator -() const { return vec2(-x, -y); }
};

vec2 operator * (float lhs, const vec2 &rhs) { return vec2(lhs * rhs.x, lhs * rhs.y); }
vec2 operator / (const vec2 &lhs, float rhs) { return vec2(lhs.x / rhs, lhs.y / rhs); }