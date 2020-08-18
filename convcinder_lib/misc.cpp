#include "StdAfx.h"
#include "misc.h"

vec3& fetch(Array2D<vec3>& src, ivec2 const& pos)
{
    static vec3 black(0.0, 0.0, 0.0);
    if(pos.x < 0 || pos.y < 0 || pos.x >= src.w || pos.y >= src.h) return black;
	return src(pos);
}



mat3 toHsv;
mat3 toHsvInv;
int initHsv()
{
	toHsv = mat3::createRotation(vec3(-1,1,0).normalized(), -acos(vec3(1,1,1).normalized().dot(vec3(0,0,1))));
	toHsvInv = toHsv.inverted();
	return 0;
}

static int ______ = initHsv();

// from the matrix.rotate method with hardcoded axis (1, 1, 1)
void rotateHue_ip( vec3& v, float angle )
{
	typedef float T;
	T sina = math<T>::sin(angle);
	T cosa1 = 1.0f - math<T>::cos(angle);
	T m = cosa1 - sina;
	T p = cosa1 + sina;

	T rx = v.x + m * v.y + p * v.z;
	T ry = p * v.x + v.y + m * v.z;
	T rz = m * v.x + p * v.y + v.z;

	v.x = rx;
	v.y = ry;
	v.z = rz;
}