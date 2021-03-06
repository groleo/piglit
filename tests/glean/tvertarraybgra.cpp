// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2009  VMware, Inc. All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL VMWARE BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT


// Render some geometry with random GLubyte/RGBA vertex colors.
// Then re-render same thing with GLubyte/BGRA vertex colors.
// Brian Paul
// 23 Jan 2009


#define GL_GLEXT_PROTOTYPES

#include <cassert>
#include <cmath>
#include <cstring>
#include "tvertarraybgra.h"
#include "rand.h"
#include "image.h"


namespace GLEAN {


VertArrayBGRAResult::VertArrayBGRAResult()
{
	pass = true;
}


void
VertArrayBGRATest::reportError(const char *msg)
{
	env->log << name << ": Error: " << msg << "\n";
}


void
VertArrayBGRATest::setupPoints()
{
	RandomDouble r(10);
	int i;
	for (i = 0; i < NUM_POINTS; i++) {
		mPos[i][0] = r.next() * WINDOW_SIZE;
		mPos[i][1] = r.next() * WINDOW_SIZE;
		mRGBA[i][0] = int(r.next() * 255);
		mRGBA[i][1] = int(r.next() * 255);
		mRGBA[i][2] = int(r.next() * 255);
		mRGBA[i][3] = int(r.next() * 255);
		mBGRA[i][0] = mRGBA[i][2];  // blue
		mBGRA[i][1] = mRGBA[i][1];  // green
		mBGRA[i][2] = mRGBA[i][0];  // red
		mBGRA[i][3] = mRGBA[i][3];  // alpha
	}
}


void
VertArrayBGRATest::renderPoints(bool useBGRA)
{
	glVertexPointer(2, GL_FLOAT, 0, mPos);
	glEnableClientState(GL_VERTEX_ARRAY);

	if (useBGRA)
		glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, 0, mBGRA);
	else
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, mRGBA);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_POINTS, 0, NUM_POINTS);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


void
VertArrayBGRATest::runOne(VertArrayBGRAResult &r, Window &w)
{
	(void) w;  // silence warning
	Image rgbaImage(WINDOW_SIZE, WINDOW_SIZE, GL_RGBA, GL_UNSIGNED_BYTE);
	Image bgraImage(WINDOW_SIZE, WINDOW_SIZE, GL_RGBA, GL_UNSIGNED_BYTE);

	setupPoints();
#if 0 // test lighting path too (debug only)
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
#endif

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WINDOW_SIZE, 0, WINDOW_SIZE, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// render with RGBA colors and save image
	glClear(GL_COLOR_BUFFER_BIT);
	renderPoints(false);
	rgbaImage.read(0, 0);  // pos=(0,0)
	w.swap();

	// render with BGRA colors and save image
	glClear(GL_COLOR_BUFFER_BIT);
	renderPoints(true);
	bgraImage.read(0, 0);  // pos=(0,0)
	w.swap();

        // images should be identical
	r.pass = (rgbaImage == bgraImage);
	if (!r.pass) {
		reportError("BGRA colors did not match RGBA colors.");
	}
}


void
VertArrayBGRATest::logOne(VertArrayBGRAResult &r)
{
	logPassFail(r);
	logConcise(r);
}


void
VertArrayBGRAResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
VertArrayBGRAResult::getresults(istream &s)
{
	char result[1000];
	s >> result;

	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
	}
	return s.good();
}


// The test object itself:
VertArrayBGRATest vertArrayBGRATest("vertArrayBGRA", "window, rgb",
			  "GL_EXT_vertex_array_bgra",
			  "Test the GL_EXT_vertex_array_bgra extension.\n");



} // namespace GLEAN


