#pragma once

#include "D.h"

const int imageSize = 200;
extern Image image;
extern float zoom;
const int windowSize = 600;
const int windowWidth = windowSize;
const int windowHeight = windowSize;
const float windowScale = windowSize / (float)imageSize;
extern Array2D<Vec4f> pic;