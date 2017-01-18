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
#include "stuff.h"
#include "shade.h"

using namespace render;
Array2D<Vec2f> vel(imageSize,Vec2f::zero());
float fluidVelocity_;
void fluid()
{
	GETFLOAT(fluidVelocity, "step=.01", .01); ::fluidVelocity_=fluidVelocity;
	GETFLOAT(fluidSteer, "step=.001", 0*.01);
	GETFLOAT(fluidGravity, "step=.0001", .0);
	Array2D<Vec3f> result(image.w, image.h, Vec3f::zero());
	auto func = [&](int yStart, int yEnd) { 
		Vec2i p;
		for(p.y = yStart; p.y < yEnd; p.y++) for(p.x = 0; p.x < image.w; p.x++)
		{
			Vec3f& c = image(p);
			Vec3f ab = toHsv.transformVec(c);
			float saturation = ab.xy().length();
			float brightness = ab.z;
			Vec2f offset = ab.xy() * (fluidVelocity * (saturation+.5));
				
			ab.rotateZ(saturation*fluidSteer);
			c=toHsvInv.transformVec(ab);
				
			offset.y += fluidGravity*(pow(brightness+saturation*2,3) + 1);
			vel(p) += offset;
				
			auto cc=c*.5;
			aaPoint(result, Vec2f(p) + vel(p), cc);
			aaPoint(result, Vec2f(p) + vel(p)/2, cc);
		}
	};
	
	boost::thread th1(func, 0, image.h / 2);
	func(image.h / 2, image.h);
	th1.join();
	auto vel2=Array2D<Vec2f>(vel.w,vel.h, Vec2f::zero());
	forxy(vel) {
		aaPoint(vel2, Vec2f(p) + vel(p), vel(p));
	}
	vel = vel2;
	image = result;
}

FFT::CArray makeKernel()
{
	static float oldSigma_ = -1, oldBo = -1, oldFd = -1;
	GETFLOAT(sigma_, "min=0 step=.1", 10); // 10
	GETFLOAT(fftDiffraction, "step=1 group=bloom'", 10);
	static FFT::CArray kernel_fs(image.w, image.h / 2 + 1); // frequency space
	if(oldSigma_ == sigma_ && oldFd == fftDiffraction)
		return kernel_fs;
	oldSigma_ = sigma_; oldFd = fftDiffraction;

	static Array2D<float> kernel(image.w, image.h);
		
	float sigmaMul = 1/(sigma_*sigma_);
	forxy(kernel)
	{
		const int radius = 10;
		static FFT::Complex one = FFT::Complex(1.0 / (pi*radius*radius), 0.0);
		static FFT::Complex zero = FFT::Complex(0.0, 0.0);
		kernel(p) = 0;
		auto func = [&](Vec2i distTo) {
			kernel(p) += exp(-p.distanceSquared(distTo)*sigmaMul);
		};
		func(Vec2i(0, 0));
		func(Vec2i(image.w, 0));
		func(Vec2i(image.w, image.h));
		func(Vec2i(0, image.h));
	}
	float sum = accumulate(kernel.begin(), kernel.end(), 0);
	float normFactor = (1.0f / image.area) / sum; // image.area because fftw produces unnormalized data
	forxy(kernel) kernel(p) *= normFactor; // #checkthis

	static auto plan3 = FFT::r2c(kernel, kernel_fs, FFTW_MEASURE);
	plan3.execute();
	return kernel_fs;
}

void genFFTBloom()
{
	FFT::CArray kernel_fs = makeKernel();

	static FFT::CArray c_plan_array(image.w, image.h / 2 + 1);
	static FFT::RArray r_plan_array(image.w, image.h);
	static auto planF = FFT::r2c(r_plan_array, c_plan_array, FFTW_MEASURE);
	static auto planB = FFT::c2r(c_plan_array, r_plan_array, FFTW_MEASURE);
		
	static FFT::CArray c_array(c_plan_array.Size());
	static FFT::RArray r_array(r_plan_array.Size());
		
	for(int channel = 0; channel < 3; channel++)
	{
		PFL(fftCopyAndMul) for(int i = 0; i < r_array.area; i++) r_array.data[i] = toDraw.data[i].ptr()[channel];
		PFL(planExecute) planF.execute(r_array, c_array);
		PFL(fftCopyAndMul) for(int i = 0; i < c_array.area; i++) comp(c_array.data[i]) *= comp(kernel_fs.data[i]);
		PFL(planExecute) planB.execute(c_array, r_array);
		forxy(r_array) r_array(p) /= (float)r_array.area;
		PFL(fftCopyAndMul) for(int i = 0; i < r_array.area; i++) bloomSurface.data[i].ptr()[channel] = r_array.data[i];
	}
	GETBOOL(showFFT, "group=fft", false);
	if(showFFT)
	{
		complexArrayToImage(kernel_fs, bloomSurface);
	}
}

void thing1()
{
	GETFLOAT(hueattract, "min=0.0 max=1.0 step=0.01", 1);
	Array2D<Vec2f> ab(image.w,image.h);
	forxy(image) {
		ab(p) = (toHsv*image(p)).xy();
	}
	forxy(image) {
		Vec2f offset = Vec2f::zero();
		Vec2f abHere = ab(p);
		for(int x=-1;x<=1;x++){
			for(int y=-1;y<=1;y++){
				Vec2f abThere = ab.wr(p.x+x,p.y+y);
				offset += Vec2f(x, y) / (1.0f - abThere.distance(abHere));
			}
		}
		::vel(p) += offset*hueattract*::fluidVelocity_*100.0;
	}
}

void post()
{
	GETFLOAT(saturationFloor, "min=0.0 max=1.0 step=0.01", 1);
	GETFLOAT(saturationMul, "min=0.0 step=0.01", 0.56);
	for(int i = 0; i < image.area; i++) {
		auto c = image.data[i];
		Vec3f hsv=rgbToHSV((Color&)c);
		float saturation = hsv.y;
		c *= saturationFloor + saturationMul * pow(saturation,2.0f);
		
		toDraw.data[i] = c;
	}
	//copy(toDraw.begin(), toDraw.end(), bloomSurface.begin());
}

void correctExposure()
{
	float sum=0.0f;
	Vec3f lumweights(.71f,.22f,.07f);
	forxy(toDraw){
		auto& c = toDraw(p);
		if(isnan_(c.x) || isnan_(c.y) || isnan_(c.z))
		{
			c = Vec3f::zero();
			continue;
		}
		//if(c.x<0||c.y<0||c.z<0)cout<<"NEG"<<endl;
		auto lum=lumweights.dot(toDraw(p));
		lum = apply(lum,[&](float f){return max(f,0.0f);});
		sum+=log(.0001f + lum);
	}
	float logavg=exp(sum/float(toDraw.area));
	static float accavg=-1.0f;
	if(accavg == -1.0f)
		accavg = logavg;
	accavg=ci::lerp(accavg,logavg,.1f);
	cout<<"accavg = "<<accavg<<endl;
	forxy(toDraw){
		toDraw(p) /= accavg;
	}
}

void updateApp()
{
	Pfl::nextFrame();
	framesElapsed++;

	input::update();
	GETBOOL(pause,"key=p",false);
	if(pause)return;

	PFL(fluid) fluid();
	PFL(thing1) thing1();
	PFL(post) post();
	correctExposure();
	PFL(genFFTBloom) genFFTBloom();
	PFL(trim) forxy(image) if(p.x < 10 || p.y < 10 || image.w-p.x < 10 || image.h - p.y < 10) image(p) = Vec3f::zero();
}

class AudioGenerativeApp : public AppBasic {
 public:
	void mouseDown(MouseEvent e) { input::mouseDown(e); }
	void mouseUp(MouseEvent e) { input::mouseUp(e); }
	void mouseMove(MouseEvent e) { input::mouseMove(e); }
	void mouseDrag(MouseEvent e) { input::mouseDrag(e); }
	void keyDown(KeyEvent e) { input::keyDown(e); }
	void keyUp(KeyEvent e) { input::keyUp(e); }
	
	void setup()
	{
		initApp();
	}

	void update()
	{
		updateApp();
		renderApp();
		GetOpt::render();
	}
};

CINDER_APP_BASIC( AudioGenerativeApp, ci::app::RendererGl )
