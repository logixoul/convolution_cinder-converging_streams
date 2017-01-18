#pragma once

#include "stdafx.h"
#include "app.h"
#include "util.h"
#include "types.h"
#include "getOpt.h"
#include "profiling.h"
#include "misc.h"
#include "fftwrap.h"
#include "D.h"
#include "expblur.h"
#include "render.h"
#include "global.h"
#include "input.h"
#include "init.h"

void complexArrayToImage(FFT::CArray in, Image& out)
{
	forxy(in)
	{
		FFT::Complex& val = in(p);
		out(Vec2i(0, out.h-1) - p) = out(p) = (Vec3f&)hsvToRGB(Vec3f(
			atan2(val.y, val.x)/twoPi+.5, // hue
			1.0, // sat
			val.length())); // value
	}
}
