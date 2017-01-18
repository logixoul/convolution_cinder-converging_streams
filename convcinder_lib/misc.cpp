#include "StdAfx.h"
#include "misc.h"

Vec3f& fetch(Array2D<Vec3f>& src, Vec2i const& pos)
{
    static Vec3f black(0.0, 0.0, 0.0);
    if(pos.x < 0 || pos.y < 0 || pos.x >= src.w || pos.y >= src.h) return black;
	return src(pos);
}



Matrix33f toHsv;
Matrix33f toHsvInv;
int initHsv()
{
	toHsv = Matrix33f::createRotation(Vec3f(-1,1,0).normalized(), -acos(Vec3f(1,1,1).normalized().dot(Vec3f(0,0,1))));
	toHsvInv = toHsv.inverted();
	return 0;
}

static int ______ = initHsv();

// from the matrix.rotate method with hardcoded axis (1, 1, 1)
void rotateHue_ip( Vec3f& v, float angle )
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