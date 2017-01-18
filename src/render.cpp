#include "StdAfx.h"
#include "render.h"
#include "global.h"
#include "misc.h"
#include "gl.h"

static ci::gl::Texture tex;
static ci::gl::Texture bloomTex;

namespace shaders
{
	static Shader a;
}

namespace render
{
	Image toDraw(imageSize,imageSize), bloomSurface(imageSize,imageSize);
	
	void init()
	{
		shaders::a = Shader("a", "a");
		gl::Texture::Format fmt;fmt.setInternalFormat(GL_RGBA16F); // faster than 16f because it doesn't have to convert on upload.
		bloomTex = gl::Texture(image.w, image.h, fmt);
		tex = gl::Texture(image.w, image.h, fmt);
	}

	//void upload2(Image& image, int buffer)
	Image clamp0(Image image) {
		auto image2 = image.clone();
		forxy(image2) {
			image2(p)=apply(image2(p), [&](float f) { return max(f,0.0f); });
		}
		return image2;
	}
	void renderApp()
	{
		glClampColor(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		glClampColor(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
		glClampColor(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);

		tex.bind();
		auto td_copy = toDraw.clone();
		auto td0 = clamp0(toDraw); // fix render artefact
		auto bl0 = clamp0(bloomSurface);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.w, image.h, GL_RGB, GL_FLOAT, &td0.data[0]);
		bloomTex.bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.w, image.h, GL_RGB, GL_FLOAT, &bl0.data[0]);

		tex.setWrap(GL_CLAMP, GL_CLAMP);
		bloomTex.setWrap(GL_CLAMP, GL_CLAMP);

		tex.bind(0);
		bloomTex.bind(1);
		glClearColor(0,0,0,0);
		gl::enableAlphaBlending();
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaders::a.id);
		shaders::a.SendConfigUniforms();
		glUniformMatrix3fv(glGetUniformLocation(shaders::a.id, "toHSL"), 1, false, toHsv);
		glUniformMatrix3fv(glGetUniformLocation(shaders::a.id, "toHSLinv"), 1, false, toHsvInv);
		glUniform1i(glGetUniformLocation(shaders::a.id, "tex"), 0);
		glUniform1i(glGetUniformLocation(shaders::a.id, "bloomTex"), 1);
		
		//GETFLOAT(bloomOpacity, "step=.001 group=bloom", .01);
		const float bloomOpacity = .01;
		glUniform1f(glGetUniformLocation(shaders::a.id, "bloomOpacity"), pow(2.0f, bloomOpacity)-1);

		glUniform2f(glGetUniformLocation(shaders::a.id, "tex_size"), image.w, image.h);
		
		glMatrixMode(GL_MODELVIEW);
		ci::gl::setMatricesWindow(windowWidth, windowHeight);
		glPushMatrix();
		glScalef(zoom,zoom,1);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(0, 0);
		glTexCoord2f(1, 0); glVertex2f(windowWidth, 0);
		glTexCoord2f(1, 1); glVertex2f(windowWidth, windowHeight);
		glTexCoord2f(0, 1); glVertex2f(0, windowHeight);
		glEnd();
		glPopMatrix();
	}
}