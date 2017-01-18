#pragma once

#include "StdAfx.h"
#include "util.h"

template<class T>
struct ListOf
{
	vector<T> data;
	ListOf(T t)
	{
		data.push_back(t);
	}
	ListOf<T>& operator()(T t)
	{
		data.push_back(t);
		return *this;
	}
	operator vector<T>()
	{
		return data;
	}
};

template<class T>
ListOf<T> list_of(T t)
{
	return ListOf<T>(t);
}

struct Str {
	string s;
	Str& operator<<(string s2) {
		s += s2 + "\n";
		return *this;
	}
	Str& operator<<(Str s2) {
		s += s2.s + "\n";
		return *this;
	}
	operator const char*() {
		return s.c_str();
	}
};

extern map<string, float> globaldict;

gl::Texture shade(vector<gl::Texture> texv, const char* fshader_constChar, float resScale = 1.0f);
namespace {
	gl::Texture::Format get_gtexfmt() { gl::Texture::Format f; f.setInternalFormat(GL_RGB32F); return f; };
	gl::Texture::Format gtexfmt = get_gtexfmt();
}
inline gl::Texture gtex(Array2D<float> a)
{
	gl::Texture tex(a.w, a.h, gtexfmt);
	tex.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, a.w, a.h, GL_LUMINANCE, GL_FLOAT, a.data);
	return tex;
}
/*gl::Texture gtex(Array2D<Vec2f> a)
{
	gl::Texture tex(a.w, a.h, gtexfmt);
	tex.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, a.w, a.h, GL_RG, GL_FLOAT, a.data);
	return tex;
}*/
inline gl::Texture gtex(Array2D<Vec3f> a)
{
	gl::Texture tex(a.w, a.h, gtexfmt);
	tex.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, a.w, a.h, GL_RGB, GL_FLOAT, a.data);
	return tex;
}