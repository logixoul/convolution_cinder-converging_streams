#pragma once

#include "StdAfx.h"
#include "util.h"

Vec3f& fetch(Array2D<Vec3f>& src, Vec2i const& pos);
template<class T>
void aaPoint(Array2D<T>& dest, Vec2f const& pos, T const& c) {
	int x = (int)pos.x;
	int y = (int)pos.y;
	float u_ratio = pos.x - x;
	float v_ratio = pos.y - y;

	if(x<0||y<0 || x>=dest.w-1 || y>=dest.h-1)
		return;
	float uv = u_ratio * v_ratio;
	float Uv = v_ratio - uv; // ((1-uratio) * v_ratio)
	float uV = u_ratio - uv; // ((1-vratio) * u_ratio)
	float UV = 1 - u_ratio - v_ratio + uv; // (1-uratio) * (1-vratio)
	auto addr = &dest(x, y);
	addr[0] += UV * c;
	addr[1] += uV * c;
	addr[dest.w] += Uv * c;
	addr[dest.w + 1] += uv * c;
}
extern Matrix33f toHsv;
extern Matrix33f toHsvInv;

static const float getSaturation_mul = abs(-Vec3f(1, 1, 1).normalized().x);
// untested, fast. ok, now tested, seems to work.
// inline for speed
inline float getSaturation(Vec3f const& v)
{
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	//static const Vec3f mul = -Vec3f(1, 1, 1).normalized();
	//return v.cross(mul).length();
	return Vec3f(v.y - v.z, v.z - v.x, v.x - v.y).length() * getSaturation_mul;
}

void rotateHue_ip(Vec3f& v, float angle);

struct HsvStruct
{
	Vec2f transform(Vec3f const& v)
	{
		return Vec2f(toHsv.m00*v.x + toHsv.m01*v.y + toHsv.m02*v.z, toHsv.m10*v.x + toHsv.m11*v.y + toHsv.m12*v.z);
	}
};
